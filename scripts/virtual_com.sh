#socat PTY,link=/tmp/ttyV0,raw PTY,link=/tmp/ttyV1,raw
#socat PTY,link=/tmp/ttyV0,raw,echo=0 PTY,link=/tmp/ttyV1,raw,echo=0
#socat -d -d PTY,link=/tmp/ttyV0,raw PTY,link=/tmp/ttyV1,raw

socat PTY,link=/tmp/ttyV0,raw PTY,link=/tmp/ttyV1,raw


