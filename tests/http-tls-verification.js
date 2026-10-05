const assert = require('node:assert/strict');
const https = require('node:https');
const fs = require('node:fs');
const { spawn } = require('node:child_process');

const [binary, keyPath, certPath] = process.argv.slice(2);
if (!binary || !keyPath || !certPath) throw new Error('usage: http-tls-verification.js BINARY KEY CERT');

const server = https.createServer({
  key: fs.readFileSync(keyPath),
  cert: fs.readFileSync(certPath),
}, (_request, response) => {
  response.writeHead(200, { 'content-type': 'text/plain' });
  response.end('untrusted certificate was accepted');
});

function runProbe(url) {
  const env = Object.fromEntries(Object.entries(process.env)
    .filter(([name]) => !/^(https?_proxy|all_proxy|no_proxy)$/i.test(name)));
  return new Promise((resolve, reject) => {
    const child = spawn(binary, [url], { env, stdio: ['ignore', 'pipe', 'pipe'] });
    let stdout = '';
    let stderr = '';
    const timer = setTimeout(() => child.kill('SIGKILL'), 15000);
    child.stdout.setEncoding('utf8').on('data', (chunk) => { stdout += chunk; });
    child.stderr.setEncoding('utf8').on('data', (chunk) => { stderr += chunk; });
    child.once('error', reject);
    child.once('close', (status) => {
      clearTimeout(timer);
      resolve({ status, stdout, stderr });
    });
  });
}

server.listen(0, '127.0.0.1', async () => {
  const address = server.address();
  try {
    const result = await runProbe(`https://127.0.0.1:${address.port}/`);
    if (result.error) throw result.error;
    assert.equal(result.status, 0, `native probe failed: ${result.stderr}`);
    assert.match(result.stdout.trim(), /^TLS_VERIFY_REJECTED:/,
      `self-signed TLS certificate must be rejected; stdout=${result.stdout} stderr=${result.stderr}`);
    assert.match(result.stdout, /certificate|SSL peer/i,
      `expected certificate-validation error, got: ${result.stdout}`);
    console.log('HTTP_TLS_VERIFICATION=PASS self-signed-certificate=rejected');
  } catch (error) {
    console.error(error);
    process.exitCode = 1;
  } finally {
    server.close();
  }
});
