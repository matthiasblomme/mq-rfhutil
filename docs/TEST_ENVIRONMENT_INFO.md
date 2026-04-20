# Test Environment Information

## Available Test Queue Manager

### Connection Details

**Queue Manager:** QM1 (Docker container)
**Host:** localhost
**Port:** 1414
**Channel:** DEV.APP.SVRCONN
**Transport:** TCP

#### Authentication (Secured Setup)
**Username:** mqapp
**Password:** securePassword123

#### Available Queues
- **TEST.HELLO.WORLD.Q1** - Test queue (from screenshot)
- **DEV.INPUT.QUEUE** - Input queue
- **DEV.OUTPUT.QUEUE** - Output queue

#### Connection Strings

**Without Authentication:**
```
DEV.APP.SVRCONN/TCP/localhost
```

**With Authentication (Recommended):**
```
Queue Manager: QM1
Channel: DEV.APP.SVRCONN
Host: localhost
Port: 1414
User: mqapp
Password: securePassword123
```

### Screenshot Analysis

From the RFHUtil screenshot, I can see:

1. **Successfully Connected:**
   - Queue Manager: `DEV.APP.SVRCONN/TCP/localhost`
   - Queue Name: `TEST.HELLO.WORLD.Q1`
   - Remote Queue Manager: `QM1`
   - Queue Type: Local
   - Queue Depth: 0 (empty)

2. **Connection Method:**
   - Using client channel: `DEV.APP.SVRCONN`
   - Transport: TCP/IP
   - Host: localhost (Docker container)
   - Port: 1414
   - Authentication: User ID and password (mqapp/securePassword123)

3. **Application Status:**
   - Multiple "No messages in queue" entries at 14:14:40-41 2033
   - Connection is active and working
   - Can browse queue successfully

### Test Environment Setup

This provides an excellent test environment for:

1. **Testing Connection Improvements:**
   - Can test HeartBeat/KeepAlive configuration
   - Can simulate connection failures
   - Can verify automatic reconnection

2. **Testing Message Operations:**
   - Put messages to TEST.HELLO.WORLD.Q1
   - Get messages from queue
   - Browse messages
   - Test various message formats

3. **Testing Channel Configuration:**
   - Verify channel settings
   - Test SSL/TLS (if configured)
   - Test different connection options

### Docker Container Information

**Actual Setup:**
```bash
# IBM MQ Docker setup with authentication
docker run -d \
  --name qm1 \
  -p 1414:1414 \
  -p 9443:9443 \
  -e LICENSE=accept \
  -e MQ_QMGR_NAME=QM1 \
  -e MQ_APP_PASSWORD=securePassword123 \
  ibmcom/mq:latest
```

**Configured Channels:**
- `DEV.APP.SVRCONN` - Application channel (with authentication)
- `DEV.ADMIN.SVRCONN` - Admin channel

**Configured Queues:**
- `TEST.HELLO.WORLD.Q1` - Test queue
- `DEV.INPUT.QUEUE` - Input queue for testing
- `DEV.OUTPUT.QUEUE` - Output queue for testing
- `DEV.QUEUE.1`, `DEV.QUEUE.2`, `DEV.QUEUE.3` - Default queues

**Authentication:**
- User: `mqapp`
- Password: `securePassword123`
- Authentication Type: User ID and Password (MQCSP)

### Testing Recommendations

#### 1. Test Current Behavior (Baseline)

**Test HeartBeat/KeepAlive defaults:**
```
1. Connect to QM1 via DEV.APP.SVRCONN/TCP/localhost
   - Use credentials: mqapp / securePassword123
2. Leave connection idle for 5+ minutes
3. Observe if connection stays alive
4. Try to put/get message to DEV.INPUT.QUEUE after idle period
5. Document any timeouts or failures
```

**RFHUtil Connection Setup:**
```
1. Open RFHUtil
2. Queue Manager Name: QM1
3. Click "Set Conn Id" button
4. Enter User ID: mqapp
5. Enter Password: securePassword123
6. Click OK
7. Click "Connect" (or use MQ menu → MQCONN)
8. Select Queue: DEV.INPUT.QUEUE or DEV.OUTPUT.QUEUE
```

**Expected Results (Current Code):**
- HeartBeat: 300s (5 minutes) - default
- KeepAlive: OS default (~7200s / 2 hours)
- May experience firewall timeouts if any

#### 2. Test Improved Configuration

**After adding HeartBeat/KeepAlive:**
```cpp
cd.HeartBeatInterval = 60;
cd.KeepAliveInterval = MQKAI_AUTO;
```

**Test Procedure:**
```
1. Rebuild RFHUtil with new configuration
2. Connect to QM1
3. Leave idle for 5+ minutes
4. Verify heartbeats are sent (check MQ logs)
5. Verify connection stays alive
6. Test message operations after idle period
```

**Expected Results:**
- HeartBeat every 60 seconds
- Connection stays alive indefinitely
- No timeouts
- Fast failure detection if QM stops

#### 3. Test Automatic Reconnection

**Simulate Connection Failure:**
```
1. Connect to QM1 with credentials (mqapp/securePassword123)
2. Put a test message to DEV.INPUT.QUEUE
3. Stop Docker container: docker stop qm1
4. Observe RFHUtil behavior (should detect failure in ~60s)
5. Start container: docker start qm1
6. Verify automatic reconnection
7. Try to get message from DEV.INPUT.QUEUE
```

**Expected Results (After Implementation):**
- Connection failure detected within 60s
- Automatic retry with backoff (1s, 2s, 4s)
- Successful reconnection when QM available
- User sees minimal disruption

#### 4. Test Network Failure

**Simulate Network Issue:**
```
1. Connect to QM1 with credentials
2. Put message to DEV.OUTPUT.QUEUE
3. Pause Docker container: docker pause qm1
4. Observe detection time (should be ~60s with improvements)
5. Unpause: docker unpause qm1
6. Verify recovery and reconnection
7. Verify message is still in queue
```

**Expected Results:**
- KeepAlive detects network failure
- Connection closed cleanly
- Automatic reconnection attempted
- Success when network restored

### MQ Configuration to Check

**Query Channel Settings:**
```bash
# Connect to QM1 container
docker exec -it qm1 bash

# Run MQSC commands
echo "DISPLAY CHANNEL(DEV.APP.SVRCONN) HBINT KAINT" | runmqsc QM1

# Check authentication settings
echo "DISPLAY CHANNEL(DEV.APP.SVRCONN) ALL" | runmqsc QM1

# Check queue definitions
echo "DISPLAY QUEUE(DEV.INPUT.QUEUE)" | runmqsc QM1
echo "DISPLAY QUEUE(DEV.OUTPUT.QUEUE)" | runmqsc QM1
echo "DISPLAY QUEUE(TEST.HELLO.WORLD.Q1)" | runmqsc QM1
```

**Expected Output:**
```
DISPLAY CHANNEL(DEV.APP.SVRCONN)
...
HBINT(300)        # Default HeartBeat interval
KAINT(AUTO)       # Default KeepAlive interval
...
```

**Modify Channel for Testing (Optional):**
```bash
# Connect to container
docker exec -it qm1 bash

# Run MQSC
runmqsc QM1

# Set aggressive heartbeat on server side
ALTER CHANNEL(DEV.APP.SVRCONN) CHLTYPE(SVRCONN) HBINT(30)

# Or disable to test client-side control
ALTER CHANNEL(DEV.APP.SVRCONN) CHLTYPE(SVRCONN) HBINT(0)

# Verify changes
DISPLAY CHANNEL(DEV.APP.SVRCONN) HBINT KAINT

# Exit MQSC
END
```

### Test Scenarios Matrix

| Scenario | Client HBINT | Server HBINT | Expected Result |
|----------|--------------|--------------|-----------------|
| **Baseline** | Not set (0) | 300 | 300s heartbeat |
| **Client Control** | 60 | 300 | 60s heartbeat (client wins) |
| **Server Control** | 60 | 30 | 30s heartbeat (server wins) |
| **Client Aggressive** | 30 | 300 | 30s heartbeat |
| **Both Disabled** | 0 | 0 | No heartbeat (risky!) |

### Monitoring and Verification

**Check MQ Logs:**
```bash
# View queue manager logs
docker exec qm1 cat /var/mqm/qmgrs/QM1/errors/AMQERR01.LOG

# Look for:
# - AMQ9208: HeartBeat interval messages
# - AMQ9209: KeepAlive interval messages
# - AMQ9999: Channel status messages
```

**Enable MQ Trace (if needed):**
```bash
# In container
strmqtrc -m QM1 -t all

# Reproduce issue

# Stop trace
endmqtrc -m QM1

# View trace
dmpmqtrc /var/mqm/trace/*.TRC
```

**RFHUtil Trace:**
```cmd
# Set environment variable
set RFHUTIL_TRACE_FILE=c:\temp\rfhutil.log

# Run RFHUtil
rfhutilc.exe

# Check log file for connection details
```

### Performance Baseline

**Measure Current Performance:**
```
1. Connect to QM1 with credentials (mqapp/securePassword123)
2. Select queue: DEV.INPUT.QUEUE
3. Put 1000 messages (use Load Q feature)
4. Record time
5. Get 1000 messages (use Display Q feature)
6. Record time
7. Document baseline

Test with different message sizes:
- Small: 100 bytes
- Medium: 10 KB
- Large: 100 KB
```

**After Improvements:**
```
1. Repeat same test
2. Compare performance
3. Verify no regression
4. Document any improvements
```

### Integration Testing Checklist

**Authentication Tests:**
- [ ] Connect with valid credentials (mqapp/securePassword123)
- [ ] Connect with invalid credentials (verify error handling)
- [ ] Connect without credentials (verify error)

**Basic Operations:**
- [ ] Put message to DEV.INPUT.QUEUE
- [ ] Get message from DEV.INPUT.QUEUE
- [ ] Put message to DEV.OUTPUT.QUEUE
- [ ] Get message from DEV.OUTPUT.QUEUE
- [ ] Put message to TEST.HELLO.WORLD.Q1
- [ ] Browse messages in all queues

**Connection Stability:**
- [ ] Connection idle for 5+ minutes (with auth)
- [ ] Connection recovery after QM restart
- [ ] Connection recovery after network pause
- [ ] HeartBeat verification (logs)
- [ ] KeepAlive verification (logs)

**Reconnection Tests:**
- [ ] Automatic reconnection test
- [ ] Multiple reconnection attempts
- [ ] Exponential backoff verification
- [ ] Reconnection with authentication

**Performance:**
- [ ] Performance comparison (before/after)
- [ ] Performance with authentication overhead
- [ ] Bulk message operations (1000+ messages)

**Error Handling:**
- [ ] Invalid queue name
- [ ] Invalid credentials
- [ ] Network timeout
- [ ] QM unavailable

### Notes for Implementation

**Connection String Format:**
```
Queue Manager Name: QM1
Channel: DEV.APP.SVRCONN
Transport: TCP
Host: localhost
Port: 1414
User: mqapp
Password: securePassword123
```

**MQSERVER Environment Variable:**
```cmd
set MQSERVER=DEV.APP.SVRCONN/TCP/localhost(1414)
```

**Authentication in RFHUtil:**
The connection uses MQCSP (Connection Security Parameters) for authentication:
```cpp
// In DataArea.cpp connect2QM method
MQCSP csp = {MQCSP_DEFAULT};
csp.AuthenticationType = MQCSP_AUTH_USER_ID_AND_PWD;
csp.CSPUserIdPtr = "mqapp";
csp.CSPPasswordPtr = "securePassword123";
cno.SecurityParmsPtr = &csp;
```

**Alternative Connection Methods:**
```
1. MQSERVER environment variable (no auth)
2. Client Channel Definition Table (CCDT) with auth
3. Direct connection in RFHUtil with "Set Conn Id" (recommended)
4. Environment variables for credentials (not recommended for security)
```

**Security Notes:**
- ✅ Always use "Set Conn Id" dialog for credentials
- ✅ Credentials are stored in memory only (not persisted)
- ✅ Use MQCSP for authentication (modern method)
- ❌ Don't hardcode credentials in code
- ❌ Don't use environment variables for passwords

### Docker Commands Reference

**Useful Commands:**
```bash
# Check if QM1 is running
docker ps | grep qm1

# View QM1 logs
docker logs qm1

# Stop QM1 (graceful)
docker stop qm1

# Start QM1
docker start qm1

# Restart QM1
docker restart qm1

# Pause QM1 (simulate network failure)
docker pause qm1

# Unpause QM1
docker unpause qm1

# Execute commands in QM1
docker exec -it qm1 bash

# Check MQ version
docker exec qm1 dspmqver
```

### Expected Test Results Summary

**Current Behavior:**
- ✅ Can connect to QM1 with authentication
- ✅ Can put/get messages to DEV.INPUT.QUEUE and DEV.OUTPUT.QUEUE
- ✅ Authentication working (mqapp/securePassword123)
- ⚠️ Slow failure detection (300s)
- ⚠️ No automatic reconnection
- ⚠️ Potential firewall timeouts
- ⚠️ Must manually reconnect after failure

**After Improvements:**
- ✅ Can connect to QM1 with authentication
- ✅ Can put/get messages to all queues
- ✅ Authentication working seamlessly
- ✅ Fast failure detection (60s)
- ✅ Automatic reconnection (with re-authentication)
- ✅ No firewall timeouts
- ✅ Better user experience
- ✅ Transparent recovery from failures

**Authentication Considerations:**
- Credentials must be re-supplied on reconnection
- ConnectionManager should cache credentials securely
- Automatic reconnection must include authentication

---

**Document Version:** 1.0  
**Date:** 2026-02-13  
**Author:** IBM Bob (Architect Mode)  
**Status:** Test Environment Documented