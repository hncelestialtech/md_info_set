/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: --
 * Date: 2019-12-17
 */

#ifndef STD_OPTION_ENUM_TYPE_H
#define STD_OPTION_ENUM_TYPE_H

namespace cffex {
namespace strategy {

// following are instrument params
#define INSTRUMENT_QUOTE_SWITCH  "Quoting"
#define INSTRUMENT_BID_OFFSET "BidOffset"
#define INSTRUMENT_ASK_OFFSET "AskOffset"

// following are instrument group params
#define PRODUCT_NETPOS_LIMIT "NetPosLimit"
#define PRODUCT_UPDATE_THRESHOLD "UpdateThreshold"
#define SERIAL_QUOTE_TEMPLATE "QuoteTemplate"

// following are local params
#define RECANCEL_INTERVAL 1000    // Unit (ms)

typedef enum {
    INVALID_QUOTING_PERIOD,
    IN_QUOTING_PERIOD,
    OUT_QUOTING_PERIOD
} quoting_period_status;

typedef enum  {
    MARKET_BASE_MODE,
    THEO_BASE_MODE
} base_mode_type;

}
}

#endif