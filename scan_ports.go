package main
import (
    "context"
    "fmt"
    "net"
    "os"
    "os/exec"
    "strconv"
    "strings"
    "sync"
    "time"
    "golang.org/x/sync/semaphore"
)

const DEFAULT_START_PORT = 0
const DEFAULT_END_PORT = 65535

type PortScanner struct {
    ip   string
    lock *semaphore.Weighted
}

func Ulimit() int64 {
    out, err := exec.Command("ulimit", "-n").Output()
    if err != nil {
        panic(err)
    }
    s := strings.TrimSpace(string(out))
    i, err := strconv.ParseInt(s, 10, 64)
    
    if err != nil {
        panic(err)
    }
    return i
}

func ScanPort(ip string, port int, timeout time.Duration) {
    target := fmt.Sprintf("%s:%d", ip, port)
    conn, err := net.DialTimeout("tcp", target, timeout)
    
    if err != nil {
        if strings.Contains(err.Error(), "too many open files") {
            time.Sleep(timeout)
            ScanPort(ip, port, timeout)
        } else {
            // fmt.Println(port, "closed")
        }
        return
    }
    
    conn.Close()
    fmt.Println(port, "open")
}

func (ps *PortScanner) Start(f, l int, timeout time.Duration) {
    wg := sync.WaitGroup{}
    defer wg.Wait()
    
    for port := f; port <= l; port++ {       
        wg.Add(1)
        ps.lock.Acquire(context.TODO(), 1)
        go func(port int) {
            defer ps.lock.Release(1)
            defer wg.Done()
            ScanPort(ps.ip, port, timeout)
        }(port)
    }
}

func main() {
    if (len(os.Args) < 2) {
        fmt.Println("Please specify a target IP.\nUsage: scan_ports [ip_add] [start_port] [end_port]")
        os.Exit(1)
    }
	ip := os.Args[1]

    var startPort, endPort int

    if (len(os.Args) < 4) {
        fmt.Sprintf("No start or end port specified, scanning from %d to %s", DEFAULT_START_PORT, DEFAULT_END_PORT)
        startPort = DEFAULT_START_PORT
        endPort = DEFAULT_END_PORT
    } else {

        startNum, err1 := strconv.Atoi(os.Args[2])
        endNum, err2 := strconv.Atoi(os.Args[3])

        if (err1 != nil || err2 != nil) {
            fmt.Println("Invalid format on start or end port.")
            os.Exit(1)
        }
        startPort = startNum
        endPort = endNum
    }

    ps := &PortScanner{
        ip:   ip,
        lock: semaphore.NewWeighted(Ulimit()),
    }
    ps.Start(startPort, endPort, 1000*time.Millisecond)
}