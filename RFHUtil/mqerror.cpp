#include "stdafx.h"
#include "mqerror.h"

// Lookup of IBM MQ reason codes to short descriptions. Values are the
// raw numeric MQRC_* constants from cmqc.h. We use raw numbers (not the
// MQRC_ symbols) so this translation unit doesn't pull in cmqc.h, which
// lets the unit-test project (which deliberately blocks MQ headers via
// prelude.h) link against this file.

const char* mqReasonString(long rc)
{
    switch (rc)
    {
    case    0: return "No error";                                      // MQRC_NONE
    case 2001: return "Alias base queue type error";                   // MQRC_ALIAS_BASE_Q_TYPE_ERROR
    case 2002: return "Already connected";                             // MQRC_ALREADY_CONNECTED
    case 2003: return "Backed out";                                    // MQRC_BACKED_OUT
    case 2004: return "Buffer error";                                  // MQRC_BUFFER_ERROR
    case 2005: return "Buffer length error";                           // MQRC_BUFFER_LENGTH_ERROR
    case 2006: return "Character attribute length error";              // MQRC_CHAR_ATTR_LENGTH_ERROR
    case 2007: return "Character attributes error";                    // MQRC_CHAR_ATTRS_ERROR
    case 2008: return "Character attributes too short";                // MQRC_CHAR_ATTRS_TOO_SHORT
    case 2009: return "Connection to queue manager lost";              // MQRC_CONNECTION_BROKEN
    case 2010: return "Data length error";                             // MQRC_DATA_LENGTH_ERROR
    case 2011: return "Dynamic queue name error";                      // MQRC_DYNAMIC_Q_NAME_ERROR
    case 2012: return "Environment error";                             // MQRC_ENVIRONMENT_ERROR
    case 2013: return "Expiry error";                                  // MQRC_EXPIRY_ERROR
    case 2014: return "Feedback error";                                // MQRC_FEEDBACK_ERROR
    case 2016: return "Get inhibited on queue";                        // MQRC_GET_INHIBITED
    case 2017: return "No more handles available";                     // MQRC_HANDLE_NOT_AVAILABLE
    case 2018: return "Connection handle not valid";                   // MQRC_HCONN_ERROR
    case 2019: return "Object handle not valid";                       // MQRC_HOBJ_ERROR
    case 2020: return "Inhibit value error";                           // MQRC_INHIBIT_VALUE_ERROR
    case 2024: return "Syncpoint limit reached";                       // MQRC_SYNCPOINT_LIMIT_REACHED
    case 2025: return "Maximum connections limit reached";             // MQRC_MAX_CONNS_LIMIT_REACHED
    case 2026: return "Message descriptor error";                      // MQRC_MD_ERROR
    case 2027: return "Missing reply-to queue";                        // MQRC_MISSING_REPLY_TO_Q
    case 2029: return "Message type error";                            // MQRC_MSG_TYPE_ERROR
    case 2030: return "Message too big for queue";                     // MQRC_MSG_TOO_BIG_FOR_Q
    case 2031: return "Message too big for queue manager";             // MQRC_MSG_TOO_BIG_FOR_Q_MGR
    case 2033: return "No message available";                          // MQRC_NO_MSG_AVAILABLE
    case 2034: return "No message under cursor";                       // MQRC_NO_MSG_UNDER_CURSOR
    case 2035: return "Not authorized";                                // MQRC_NOT_AUTHORIZED
    case 2036: return "Queue not open for browse";                     // MQRC_NOT_OPEN_FOR_BROWSE
    case 2037: return "Queue not open for input";                      // MQRC_NOT_OPEN_FOR_INPUT
    case 2038: return "Queue not open for inquire";                    // MQRC_NOT_OPEN_FOR_INQUIRE
    case 2039: return "Queue not open for output";                     // MQRC_NOT_OPEN_FOR_OUTPUT
    case 2041: return "Object definition changed since opened";        // MQRC_OBJECT_CHANGED
    case 2042: return "Object in use";                                 // MQRC_OBJECT_IN_USE
    case 2043: return "Option not valid for object type";              // MQRC_OPTION_NOT_VALID_FOR_TYPE
    case 2046: return "Options error";                                 // MQRC_OPTIONS_ERROR
    case 2047: return "Persistence error";                             // MQRC_PERSISTENCE_ERROR
    case 2048: return "Persistent messages not allowed";               // MQRC_PERSISTENT_NOT_ALLOWED
    case 2049: return "Priority exceeds maximum";                      // MQRC_PRIORITY_EXCEEDS_MAXIMUM
    case 2050: return "Priority error";                                // MQRC_PRIORITY_ERROR
    case 2051: return "Put inhibited on queue";                        // MQRC_PUT_INHIBITED
    case 2052: return "Queue deleted";                                 // MQRC_Q_DELETED
    case 2053: return "Queue full";                                    // MQRC_Q_FULL
    case 2056: return "Queue space not available";                     // MQRC_Q_SPACE_NOT_AVAILABLE
    case 2057: return "Queue type error";                              // MQRC_Q_TYPE_ERROR
    case 2058: return "Queue manager name error";                      // MQRC_Q_MGR_NAME_ERROR
    case 2059: return "Queue manager not available";                   // MQRC_Q_MGR_NOT_AVAILABLE
    case 2061: return "Report options error";                          // MQRC_REPORT_OPTIONS_ERROR
    case 2063: return "Security error";                                // MQRC_SECURITY_ERROR
    case 2071: return "Storage not available";                         // MQRC_STORAGE_NOT_AVAILABLE
    case 2072: return "Syncpoint not available";                       // MQRC_SYNCPOINT_NOT_AVAILABLE
    case 2079: return "Truncated message accepted";                    // MQRC_TRUNCATED_MSG_ACCEPTED
    case 2080: return "Truncated message failed";                      // MQRC_TRUNCATED_MSG_FAILED
    case 2085: return "Unknown object name";                           // MQRC_UNKNOWN_OBJECT_NAME
    case 2087: return "Unknown remote queue manager";                  // MQRC_UNKNOWN_REMOTE_Q_MGR
    case 2090: return "Wait interval error";                           // MQRC_WAIT_INTERVAL_ERROR
    case 2092: return "Cluster exit error";                            // MQRC_CLUSTER_EXIT_ERROR
    case 2093: return "Queue not open for pass all";                   // MQRC_NOT_OPEN_FOR_PASS_ALL
    case 2094: return "Queue not open for pass identity";              // MQRC_NOT_OPEN_FOR_PASS_IDENT
    case 2095: return "Queue not open for set all";                    // MQRC_NOT_OPEN_FOR_SET_ALL
    case 2096: return "Queue not open for set identity";               // MQRC_NOT_OPEN_FOR_SET_IDENT
    case 2099: return "Signal request accepted";                       // MQRC_SIGNAL_REQUEST_ACCEPTED
    case 2100: return "Resource problem";                              // MQRC_RESOURCE_PROBLEM (alias)
    case 2101: return "Object damaged";                                // MQRC_OBJECT_DAMAGED
    case 2102: return "Resource problem";                              // MQRC_RESOURCE_PROBLEM
    case 2110: return "Format error";                                  // MQRC_FORMAT_ERROR
    case 2111: return "Source CCSID error";                            // MQRC_SOURCE_CCSID_ERROR
    case 2119: return "Message data not converted";                    // MQRC_NOT_CONVERTED
    case 2120: return "Converted message too big";                     // MQRC_CONVERTED_MSG_TOO_BIG
    case 2142: return "Header error";                                  // MQRC_HEADER_ERROR
    case 2150: return "DLH error";                                     // MQRC_DLH_ERROR
    case 2152: return "Object name error";                             // MQRC_OBJECT_NAME_ERROR
    case 2161: return "Queue manager quiescing";                       // MQRC_Q_MGR_QUIESCING
    case 2162: return "Queue manager stopping";                        // MQRC_Q_MGR_STOPPING
    case 2173: return "PMO error";                                     // MQRC_PMO_ERROR
    case 2183: return "Channel auto-definition error";                 // MQRC_CHANNEL_AUTO_DEF_ERROR
    case 2189: return "Cluster resolution error";                      // MQRC_CLUSTER_RESOLUTION_ERROR
    case 2192: return "Storage medium full";                           // MQRC_STORAGE_MEDIUM_FULL
    case 2193: return "Pageset error";                                 // MQRC_PAGESET_ERROR
    case 2195: return "Unexpected error";                              // MQRC_UNEXPECTED_ERROR
    case 2196: return "Unknown transmission queue";                    // MQRC_UNKNOWN_XMIT_Q
    case 2197: return "Unknown default transmission queue";            // MQRC_UNKNOWN_DEF_XMIT_Q
    case 2199: return "Get message under cursor blocked";              // MQRC_GET_MSG_BLOCKED
    case 2202: return "Connection quiescing";                          // MQRC_CONNECTION_QUIESCING
    case 2203: return "Connection stopping";                           // MQRC_CONNECTION_STOPPING
    case 2207: return "Group identifier error";                        // MQRC_GROUP_ID_ERROR
    case 2218: return "Message too big for channel";                   // MQRC_MSG_TOO_BIG_FOR_CHANNEL
    case 2270: return "No destinations available";                     // MQRC_NO_DESTINATIONS_AVAILABLE
    case 2277: return "Channel definition (MQCD) error";               // MQRC_CD_ERROR
    case 2298: return "Function not supported";                        // MQRC_FUNCTION_NOT_SUPPORTED
    case 2305: return "Unknown function";                              // MQRC_UNKNOWN_FUNCTION (PCF)
    case 2318: return "Selector not supported";                        // MQRC_SELECTOR_NOT_SUPPORTED
    case 2334: return "RFH error";                                     // MQRC_RFH_ERROR
    case 2335: return "RFH string error";                              // MQRC_RFH_STRING_ERROR
    case 2336: return "RFH command error";                             // MQRC_RFH_COMMAND_ERROR
    case 2337: return "RFH parameter error";                           // MQRC_RFH_PARM_ERROR
    case 2338: return "RFH duplicate parameter";                       // MQRC_RFH_DUPLICATE_PARM
    case 2339: return "RFH parameter missing";                         // MQRC_RFH_PARM_MISSING
    case 2354: return "Unit of work not available";                    // MQRC_UOW_NOT_AVAILABLE
    case 2371: return "Subscription option error";                     // MQRC_SUBSCRIPTION_OPTION_ERROR
    case 2381: return "Key repository error";                          // MQRC_KEY_REPOSITORY_ERROR
    case 2393: return "SSL configuration error";                       // MQRC_SSL_CONFIG_ERROR
    case 2394: return "SSL initialization error";                      // MQRC_SSL_INITIALIZATION_ERROR
    case 2396: return "SSL not allowed";                               // MQRC_SSL_NOT_ALLOWED
    case 2398: return "SSL peer name mismatch";                        // MQRC_SSL_PEER_NAME_MISMATCH
    case 2399: return "SSL peer name error";                           // MQRC_SSL_PEER_NAME_ERROR
    case 2400: return "SSL certificate revoked";                       // MQRC_SSL_CERTIFICATE_REVOKED
    case 2401: return "SSL certificate store error";                   // MQRC_SSL_CERT_STORE_ERROR
    case 2409: return "SSL key reset error";                           // MQRC_SSL_KEY_RESET_ERROR
    case 2417: return "Message not allowed in group";                  // MQRC_MSG_NOT_ALLOWED_IN_GROUP
    case 2424: return "Subscription descriptor error";                 // MQRC_SD_ERROR
    case 2425: return "Topic string error";                            // MQRC_TOPIC_STRING_ERROR
    case 2428: return "No subscription";                               // MQRC_NO_SUBSCRIPTION
    case 2429: return "Subscription in use";                           // MQRC_SUBSCRIPTION_IN_USE
    case 2431: return "Subscription user data error";                  // MQRC_SUB_USER_DATA_ERROR
    case 2432: return "Subscription already exists";                   // MQRC_SUB_ALREADY_EXISTS
    case 2440: return "Subscription name error";                       // MQRC_SUB_NAME_ERROR
    case 2442: return "Property name error";                           // MQRC_PROPERTY_NAME_ERROR
    case 2459: return "Selector syntax error";                         // MQRC_SELECTOR_SYNTAX_ERROR
    case 2469: return "Property type not supported";                   // MQRC_PROP_TYPE_NOT_SUPPORTED
    case 2473: return "Property type error";                           // MQRC_PROPERTY_TYPE_ERROR
    case 2502: return "Publication failure";                           // MQRC_PUBLICATION_FAILURE
    case 2515: return "Subscription inhibited";                        // MQRC_SUB_INHIBITED
    case 2520: return "Selector always false";                         // MQRC_SELECTOR_ALWAYS_FALSE
    case 2526: return "Property name length error";                    // MQRC_PROPERTY_NAME_LENGTH_ERR
    case 2528: return "Connection stopped";                            // MQRC_CONNECTION_STOPPED
    case 2531: return "Publish/subscribe inhibited";                   // MQRC_PUBSUB_INHIBITED
    case 2537: return "Channel not available";                         // MQRC_CHANNEL_NOT_AVAILABLE
    case 2538: return "Host not available";                            // MQRC_HOST_NOT_AVAILABLE
    case 2539: return "Channel configuration error";                   // MQRC_CHANNEL_CONFIG_ERROR
    case 2540: return "Unknown channel name";                          // MQRC_UNKNOWN_CHANNEL_NAME
    case 2543: return "Standby queue manager";                         // MQRC_STANDBY_Q_MGR
    case 2547: return "Selection string error";                        // MQRC_SELECTION_STRING_ERROR
    case 2557: return "Selector not alterable";                        // MQRC_SELECTOR_NOT_ALTERABLE
    case 6104: return "Attribute locked";                              // MQRC_ATTRIBUTE_LOCKED
    default:   return NULL;
    }
}
