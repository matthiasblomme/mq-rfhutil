#include "stdafx.h"
#include "MQConnection.h"

// P4.1 PR A: MQConnection is a passive header-only struct for now. This
// translation unit exists so the class has a stable home for future
// out-of-line method definitions added in PR C (connect, disconnect,
// reconnect, health monitor) without forcing a wave of recompiles.
