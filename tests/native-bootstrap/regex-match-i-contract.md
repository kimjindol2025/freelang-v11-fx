# DB95-7 fixed native contract

Add regex_match_i(text, pattern) -> boolean. Match ASCII SQL keywords without
case sensitivity and use the existing native POSIX ERE engine. Text comes first,
matching the db95/AFJ call sites. Existing regex_match(pattern, text) is unchanged.
The oracle fixes 15 new match results and 3 existing native match results.
Invalid patterns must throw with prefix "regex_match_i: invalid pattern:".
Either non-string argument must throw "regex_match_i: string arguments required".
No false result may hide these errors. The error message's engine detail is not
fixed across platforms. Native string inputs and ERE syntax are covered; full
JavaScript RegExp syntax/coercions are not claimed. Do not edit the JS runner.
