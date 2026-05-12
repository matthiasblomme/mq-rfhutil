#pragma once

// Returns a short human-readable description for an IBM MQ reason code
// (e.g. 2035 -> "Not authorized"), or NULL if the code is not mapped.
//
// Parameter is plain `long` so this header can be used from translation
// units that don't include cmqc.h (in particular the unit-test project,
// which deliberately blocks MQ headers).
const char* mqReasonString(long rc);
