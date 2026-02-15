# IBM MQ KeepAlive - Detailed Technical Explanation

## Yes! KeepAlive is Also Bidirectional

Just like HeartBeat, **TCP KeepAlive is sent by BOTH sides** of the connection. However, it works differently because it's controlled by the operating system, not MQ.

---

## How TCP KeepAlive Really Works

### 1. Operating System Control

**Key Point:** KeepAlive is managed by the **TCP/IP stack in the operating system**, not by MQ or the application.

```
Application Layer:    [MQ Client] <-----------> [MQ Queue Manager]
                           ↓                            ↓
Transport Layer:      [TCP Stack] <-----------> [TCP Stack]
                           ↓                            ↓
                      KeepAlive                    KeepAlive
                      Probes                       Probes
```

### 2. Bidirectional KeepAlive

**Both sides can send KeepAlive probes:**

```
Client OS                                    Server OS
TCP Stack                                    TCP Stack
    |                                            |
    |  --- TCP KeepAlive Probe (empty) ------>  |
    |  <--- TCP ACK --------------------------- |
    |                                            |
    |  <--- TCP KeepAlive Probe (empty) ------  |
    |  --- TCP ACK ---------------------------> |
    |                                            |
```

**Important:** Each side's TCP stack independently decides when to send probes based on:
- Its own KeepAlive configuration
- Socket activity
- Operating system settings

---

## KeepAlive Configuration in MQ

### Client-Side Configuration

```cpp
// In MQCD structure (client connection)
cd.KeepAliveInterval = MQKAI_AUTO;  // Use OS default
// or
cd.KeepAliveInterval = 30;          // 30 seconds
```

### Server-Side Configuration

```
// In channel definition (MQSC)
ALTER CHANNEL(MY.SVRCONN) CHLTYPE(SVRCONN) KAINT(AUTO)
// or
ALTER CHANNEL(MY.SVRCONN) CHLTYPE(SVRCONN) KAINT(30)
```

### What These Settings Do

**Important:** Unlike HeartBeat, these settings tell MQ to **configure the OS TCP stack**, not to send MQ-level flows.

```
MQ Client                           MQ Queue Manager
    |                                      |
    | cd.KeepAliveInterval = 30            | KAINT(60)
    ↓                                      ↓
Configure OS:                       Configure OS:
setsockopt(SO_KEEPALIVE)           setsockopt(SO_KEEPALIVE)
TCP_KEEPIDLE = 30s                 TCP_KEEPIDLE = 60s
    ↓                                      ↓
Client OS TCP Stack                Server OS TCP Stack
Sends probes every 30s             Sends probes every 60s
```

---

## KeepAlive Negotiation (Different from HeartBeat!)

### No Negotiation - Each Side Independent

**Unlike HeartBeat, there is NO negotiation for KeepAlive!**

Each side configures its own TCP stack independently:

```
Client: KAINT(30)  → Client OS sends probes every 30s
Server: KAINT(60)  → Server OS sends probes every 60s

Result: BOTH intervals are active simultaneously!
```

### Example Scenario

```
Time 0:00 - Connection established
Time 0:30 - Client OS sends KeepAlive probe
Time 0:30 - Server OS responds with ACK
Time 1:00 - Client OS sends KeepAlive probe
Time 1:00 - Server OS sends KeepAlive probe (its 60s interval)
Time 1:00 - Both respond with ACKs
Time 1:30 - Client OS sends KeepAlive probe
Time 2:00 - Client OS sends KeepAlive probe
Time 2:00 - Server OS sends KeepAlive probe
...and so on
```

**Key Point:** The connection is kept alive by **whichever side sends probes more frequently**.

---

## Operating System Defaults

### Windows

**Default Settings (if KAINT=AUTO):**

```
TCP KeepAlive Time:     7200 seconds (2 hours)
TCP KeepAlive Interval: 1 second (between retries)
TCP KeepAlive Probes:   10 retries
```

**Registry Location:**
```
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters
KeepAliveTime = 7200000 (milliseconds)
KeepAliveInterval = 1000 (milliseconds)
```

### Linux

**Default Settings:**

```
tcp_keepalive_time:   7200 seconds (2 hours)
tcp_keepalive_intvl:  75 seconds (between retries)
tcp_keepalive_probes: 9 retries
```

**Configuration Files:**
```
/proc/sys/net/ipv4/tcp_keepalive_time
/proc/sys/net/ipv4/tcp_keepalive_intvl
/proc/sys/net/ipv4/tcp_keepalive_probes
```

### AIX

**Default Settings:**

```
tcp_keepidle:  7200 seconds (2 hours)
tcp_keepintvl: 150 seconds (between retries)
tcp_keepcnt:   8 retries
```

---

## How MQ Configures KeepAlive

### What MQ Does Behind the Scenes

When you set `cd.KeepAliveInterval`, MQ calls OS socket options:

```cpp
// Pseudo-code of what MQ does internally

// Enable KeepAlive on the socket
int optval = 1;
setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

// Set KeepAlive time (idle time before first probe)
#ifdef _WIN32
    // Windows
    DWORD keepalive_time = cd.KeepAliveInterval * 1000; // Convert to ms
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(keepalive_time));
#else
    // Linux/Unix
    int keepalive_time = cd.KeepAliveInterval;
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(keepalive_time));
#endif
```

### MQKAI_AUTO Behavior

When you set `cd.KeepAliveInterval = MQKAI_AUTO`:

```cpp
if (cd.KeepAliveInterval == MQKAI_AUTO) {
    // Enable KeepAlive but use OS defaults
    setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    // Don't set TCP_KEEPIDLE - use OS default (7200s)
}
```

---

## Both Sides Send KeepAlive - Practical Impact

### Scenario 1: Asymmetric Configuration

**Setup:**
```
Client: KAINT(30)  - Aggressive
Server: KAINT(300) - Relaxed
```

**What Happens:**

```
Time 0:00 - Connection established
Time 0:30 - Client sends probe (client's 30s interval)
Time 1:00 - Client sends probe
Time 1:30 - Client sends probe
Time 2:00 - Client sends probe
Time 2:30 - Client sends probe
Time 3:00 - Client sends probe
Time 5:00 - Server sends probe (server's 300s interval)
```

**Result:** Connection is kept alive by client's more frequent probes (every 30s)

**Benefit:** Even if server has long interval, client's aggressive probes prevent connection drop

### Scenario 2: Both Sides Aggressive

**Setup:**
```
Client: KAINT(30)
Server: KAINT(30)
```

**What Happens:**

```
Time 0:00 - Connection established
Time 0:30 - Both client and server send probes (may overlap)
Time 1:00 - Both send probes
Time 1:30 - Both send probes
```

**Result:** Redundant probes, but ensures connection stays alive

**Impact:** Slightly more network traffic, but negligible (40-60 bytes per probe)

### Scenario 3: One Side Disabled

**Setup:**
```
Client: KAINT(0)   - Disabled
Server: KAINT(30)  - Enabled
```

**What Happens:**

```
Time 0:00 - Connection established
Time 0:30 - Server sends probe
Time 1:00 - Server sends probe
Time 1:30 - Server sends probe
```

**Result:** Server's probes keep connection alive

**Important:** Even if client disables KeepAlive, server's probes still work!

---

## KeepAlive Failure Detection

### How Failure is Detected

When a KeepAlive probe is sent:

```
1. OS sends TCP KeepAlive probe (empty packet)
2. Wait for ACK
3. If no ACK, retry after TCP_KEEPINTVL seconds
4. Retry TCP_KEEPCNT times
5. If all retries fail, close socket
6. Application (MQ) receives socket error
```

### Example Timeline (Windows)

**Configuration:**
```
KAINT = 30 seconds
TCP_KEEPINTVL = 1 second (Windows default)
TCP_KEEPCNT = 10 retries (Windows default)
```

**Failure Scenario:**

```
Time 0:00 - Connection established
Time 0:30 - Client sends KeepAlive probe
Time 0:30 - Network cable unplugged (probe lost)
Time 0:31 - Retry 1 (no response)
Time 0:32 - Retry 2 (no response)
Time 0:33 - Retry 3 (no response)
...
Time 0:40 - Retry 10 (no response)
Time 0:40 - OS closes socket
Time 0:40 - MQ detects socket closure
Time 0:40 - Application receives MQRC_CONNECTION_BROKEN
```

**Total detection time:** 30s (idle) + 10s (retries) = **40 seconds**

### Example Timeline (Linux)

**Configuration:**
```
KAINT = 30 seconds
tcp_keepalive_intvl = 75 seconds (Linux default)
tcp_keepalive_probes = 9 retries (Linux default)
```

**Failure Scenario:**

```
Time 0:00 - Connection established
Time 0:30 - Client sends KeepAlive probe (no response)
Time 1:45 - Retry 1 (75s later, no response)
Time 3:00 - Retry 2 (75s later, no response)
...
Time 11:30 - Retry 9 (no response)
Time 11:30 - OS closes socket
```

**Total detection time:** 30s + (9 × 75s) = **705 seconds (11.75 minutes)**

**Important:** Linux has much longer retry intervals than Windows!

---

## Best Practices for KeepAlive

### 1. Enable on Both Sides

**Recommended:**

```cpp
// Client (RFHUtil)
cd.KeepAliveInterval = MQKAI_AUTO;  // or 30

// Server (MQSC)
ALTER CHANNEL(MY.SVRCONN) KAINT(AUTO)  // or 30
```

**Why:** Redundancy - if one side fails to send, the other side still detects

### 2. Use MQKAI_AUTO for Most Cases

```cpp
cd.KeepAliveInterval = MQKAI_AUTO;
```

**Benefits:**
- Uses OS defaults (well-tested)
- Respects system-wide settings
- Less aggressive (lower overhead)
- Suitable for stable networks

### 3. Use Explicit Value for Problematic Networks

```cpp
cd.KeepAliveInterval = 30;  // 30 seconds
```

**When to use:**
- Firewalls with short idle timeouts
- Unreliable networks
- NAT devices that drop idle connections
- Load balancers with connection limits

### 4. Consider OS Differences

**Windows:** Fast retries (1s interval) = quick detection
**Linux:** Slow retries (75s interval) = slow detection

**Recommendation for Linux:**
```bash
# Reduce retry interval for faster detection
echo 10 > /proc/sys/net/ipv4/tcp_keepalive_intvl
```

---

## KeepAlive vs HeartBeat - Which Detects Faster?

### Comparison

| Scenario | HeartBeat | KeepAlive (Windows) | KeepAlive (Linux) |
|----------|-----------|---------------------|-------------------|
| **MQ Process Crash** | 60s | No detection | No detection |
| **Network Cable Unplug** | 60s | 40s (30+10) | 705s (30+675) |
| **Firewall Drop** | 60s | 40s | 705s |
| **QM Graceful Stop** | Immediate | Immediate | Immediate |

**Key Insight:** 
- **HeartBeat** is better for MQ-specific failures
- **KeepAlive (Windows)** is faster for network failures
- **KeepAlive (Linux)** is slower due to long retry intervals

### Recommendation: Use Both!

```cpp
cd.HeartBeatInterval = 60;        // MQ-level detection
cd.KeepAliveInterval = MQKAI_AUTO; // Network-level detection
```

This provides:
- ✅ Fast MQ failure detection (60s)
- ✅ Network failure detection (OS-managed)
- ✅ Redundancy (both mechanisms active)
- ✅ Firewall timeout prevention

---

## RFHUtil Specific Recommendations

### Current State

```cpp
// DataArea.cpp:10350-10450
MQCD cd = {MQCD_CLIENT_CONN_DEFAULT};
// KeepAliveInterval NOT SET
// Uses structure default = 0 (disabled!)
```

**Problem:** KeepAlive is disabled, relying only on HeartBeat (which is also not set)

### Recommended Configuration

```cpp
// Add after line 10407 in DataArea.cpp

// Set HeartBeat for MQ-level health checks
cd.HeartBeatInterval = 60;

// Set KeepAlive for network-level health checks
cd.KeepAliveInterval = MQKAI_AUTO;  // Use OS defaults

// For aggressive firewall environments:
// cd.KeepAliveInterval = 30;  // 30 seconds
```

### Why Both?

**HeartBeat (60s):**
- Detects MQ process failures
- Detects queue manager issues
- Prevents firewall timeouts
- MQ-aware detection

**KeepAlive (AUTO):**
- Detects network failures
- Detects cable unplugs
- Detects router/switch failures
- OS-level detection

**Together:** Comprehensive failure detection at both application and network layers

---

## Summary

### Key Points

1. **KeepAlive is bidirectional** - Both client and server send probes
2. **No negotiation** - Each side configures independently
3. **OS-controlled** - TCP stack manages probes, not MQ
4. **Both sides matter** - Either side can keep connection alive
5. **Platform differences** - Windows faster than Linux for detection

### Configuration Matrix

| Setting | Client | Server | Result |
|---------|--------|--------|--------|
| **Aggressive** | 30 | 30 | Both send every 30s |
| **Asymmetric** | 30 | 300 | Client keeps alive (30s) |
| **Disabled Client** | 0 | 30 | Server keeps alive (30s) |
| **Both Disabled** | 0 | 0 | No KeepAlive (risky!) |
| **AUTO** | AUTO | AUTO | OS defaults (7200s) |

### Recommended for RFHUtil

```cpp
cd.HeartBeatInterval = 60;        // Fast MQ failure detection
cd.KeepAliveInterval = MQKAI_AUTO; // Network failure detection
```

**Benefits:**
- ✅ Comprehensive failure detection
- ✅ Works on both Windows and Linux
- ✅ Respects OS settings
- ✅ Minimal overhead
- ✅ Production-ready

---

**Document Version:** 1.0  
**Date:** 2026-02-13  
**Author:** IBM Bob (Architect Mode)