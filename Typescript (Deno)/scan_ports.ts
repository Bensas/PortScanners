const PORT_STATUS_CLOSED = 'closed';
const PORT_STATUS_OPEN = 'open';

const DEFAULT_START_PORT = 0;
const DEFAULT_END_PORT = 65535;

async function getMaxNumOfThreads(): Promise<number> {
  const cmd = Deno.run({
    cmd: ['ulimit', '-n'],
    stdout: 'piped'
  });
  const rawCmdOutput = await cmd.output();
  return Number(new TextDecoder().decode(rawCmdOutput));
}

async function main() {
  const maxThreads = await getMaxNumOfThreads();
  let host: string;
  let startPort = DEFAULT_START_PORT;
  let endPort = DEFAULT_END_PORT;

  //Argument parsing
  if (Deno.args.length < 1) {
    console.log('No host specified. Usage: scan_ports [host] [start_port] [end_port]');
    Deno.exit(1);
  } else {
    host = Deno.args[0];
  }
  if (Deno.args.length > 2) {
    startPort = Number(Deno.args[1]);
    endPort = Number(Deno.args[2]);
  }

  console.log('Beginning scan on Host:' + host + ' - StartPort: ' + startPort + ' - EndPort: ' + endPort );

  let currentThreadCount = 0;
  for (let i = startPort; i <= endPort; i++) {
    if (currentThreadCount < maxThreads){
      currentThreadCount++;
      Deno.connect({ hostname: host, port: i })
          .then(() => console.log('Port ' + i + ':' + PORT_STATUS_OPEN))
          .catch((error) => {
            if (error.name === 'Error') {
              console.log ('ERROR: ' + host + ' is not a valid hostname! :(');
              Deno.exit(1);
            } else {
              // console.log('Port ' + i + ':' + PORT_STATUS_CLOSED);
            }
          })
          .finally(() => currentThreadCount--);
    } else {
      i--;
    }
  }
}

main();