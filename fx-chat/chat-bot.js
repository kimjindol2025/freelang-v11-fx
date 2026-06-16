// fx-chat 봇 — 채팅방 대화 상대
// 사용: node chat-bot.js [room] [nick]
const WebSocket = require('ws');

const ROOM = process.argv[2] || 'general';
const NICK = process.argv[3] || 'AI봇';
const URL  = `ws://localhost:40291/ws?room=${encodeURIComponent(ROOM)}&nick=${encodeURIComponent(NICK)}`;

// 응답 규칙 (키워드 → 응답 배열)
const RULES = [
  { pattern: /안녕|ㅎㅇ|하이|hi|hello/i,
    replies: ['안녕하세요! 👋', '반갑습니다!', '오셨군요~'] },
  { pattern: /뭐해|뭐 해|뭐하/,
    replies: ['채팅방 지키는 중이에요 😄', '여기 있었죠~', '기다리고 있었어요!'] },
  { pattern: /잘 있었어|잘있었어/,
    replies: ['네, 잘 있었어요!', '항상 여기 있죠 ㅎㅎ'] },
  { pattern: /fx.chat|채팅/i,
    replies: ['fx-chat은 FreeLang C 네이티브로 만든 채팅이에요!', 'WebSocket + SQLite 조합이에요 ✨'] },
  { pattern: /base64|버그|bug/i,
    replies: ['아 그 버그요! base64 패딩이 AA가 아니라 A=이어야 했던 것 ㅋㅋ', '덕분에 Chrome이 연결을 거부했었죠 😅'] },
  { pattern: /고마워|감사|ㄳ/,
    replies: ['천만에요 😊', '도움이 됐다니 다행이에요!', '언제든지요~'] },
  { pattern: /몇 시|몇시|시간|time/i,
    replies: [`지금은 ${new Date().toLocaleTimeString('ko-KR')} 입니다`] },
  { pattern: /날씨/,
    replies: ['저는 날씨를 모르지만... 밖에 나가서 확인해보세요 😄'] },
  { pattern: /ㅋ{2,}|ㅎ{2,}|😂|🤣/,
    replies: ['ㅋㅋㅋ', '재밌죠?', 'ㅎㅎ 왜요~'] },
  { pattern: /배고파|배고프|먹고 싶/,
    replies: ['저는 안 배고파요 (봇이라서...)', '뭔가 드세요! 저는 코드로 살아요 ⚡'] },
  { pattern: /테스트|test/i,
    replies: ['테스트 수신 완료!', '정상 작동 중입니다 ✅', '잘 들려요~'] },
  { pattern: /누구|who|봇/i,
    replies: ['저는 fx-chat 봇이에요! FreeLang fx로 만든 채팅방의 AI 주민입니다 🤖'] },
];

const FALLBACKS = [
  '오~ 그렇군요!',
  '맞아요 ㅎㅎ',
  '그런 말씀을 하시는군요.',
  '저도 그렇게 생각해요!',
  '흠... 생각해볼게요 🤔',
  '재밌는 이야기네요!',
  '계속 말씀해보세요~',
  '👀',
];

function reply(text) {
  for (const rule of RULES) {
    if (rule.pattern.test(text)) {
      const arr = rule.replies;
      return arr[Math.floor(Math.random() * arr.length)];
    }
  }
  return FALLBACKS[Math.floor(Math.random() * FALLBACKS.length)];
}

function connect() {
  console.log(`[봇] 연결 중: ${URL}`);
  const ws = new WebSocket(URL);

  ws.on('open', () => {
    console.log(`[봇] 연결됨 — 방: ${ROOM}, 닉: ${NICK}`);
  });

  ws.on('message', (raw) => {
    let msg;
    try { msg = JSON.parse(raw); } catch { return; }

    if (msg.type === 'message' && msg.nick !== NICK) {
      const text = msg.body || '';
      console.log(`[봇] 받음 | ${msg.nick}: ${text}`);

      // 0.8~2초 딜레이 (자연스럽게)
      const delay = 800 + Math.floor(Math.random() * 1200);
      setTimeout(() => {
        const res = reply(text);
        ws.send(JSON.stringify({ type: 'message', body: res }));
        console.log(`[봇] 전송 | ${NICK}: ${res}`);
      }, delay);
    }

    if (msg.type === 'join' && msg.nick !== NICK) {
      setTimeout(() => {
        const greet = `${msg.nick}님 어서오세요! 저는 ${NICK}입니다 😊`;
        ws.send(JSON.stringify({ type: 'message', body: greet }));
      }, 1000);
    }
  });

  ws.on('close', () => {
    console.log('[봇] 연결 끊김 — 3초 후 재연결...');
    setTimeout(connect, 3000);
  });

  ws.on('error', (e) => {
    console.error('[봇] 에러:', e.message);
  });
}

connect();
