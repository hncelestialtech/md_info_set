/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: renjh
 * Date: 2022-05-16
 */

#ifndef FB_MD_TYPE_H
#define FB_MD_TYPE_H

#include <stddef.h>

namespace cffex {
namespace fb {
namespace api {

// exchange_id_type
enum fb_exchange_type {
    FB_EXCHANGE_CFFEX   = '0', /* 中金所 */
    FB_EXCHANGE_SHFE    = '1', /* 上期所 */
    FB_EXCHANGE_DCE     = '2', /* 大商所 */
    FB_EXCHANGE_ZCE     = '3', /* 郑商所 */
    FB_EXCHANGE_SSE     = '4', /* 上交所 */
    FB_EXCHANGE_SZSE    = '5', /* 深交所 */
    FB_EXCHANGE_INE     = '6', /* 能源交易中心 */
    FB_EXCHANGE_GFEX    = '7', /* 广期所 */
    FB_EXCHANGE_BSE     = '8', /* 北交所 */
    FB_EXCHANGE_UNKNOWN = 'n',
    FB_EXCHANGE_ALL     = 'z'
};

// inquiry_quote_status_type
enum {
    FB_INQUIRY_QUOTE_WAITING = '0', /* 等待回应 */
    FB_INQUIRY_QUOTE_FINISH  = '1', /* 已回应 */
    FB_INQUIRY_QUOTE_TIMEOUT = '2'  /* 回应超时 */
};

// instrument_trading_status_type
enum {
    FB_UNKNOWN                 = '0', /* 未知状态 */
    FB_BEFORE_TRADING          = '1', /* 开盘前 */
    FB_NOTRADING               = '2', /* 非交易 */
    FB_CONTINOUS               = '3', /* 连续交易 */
    FB_AUCTION_ORDERING        = '4', /* 集合竞价报单 */
    FB_AUCTION_MATCH           = '5', /* 集合竞价撮合 */
    FB_CLOSED                  = '6', /* 收盘 */
    FB_SUSPENDED               = '7', /* 停牌 */
    FB_CIRCUIT_BREAKER         = '8', /* 熔断 */
    FB_VOLATILITY_DISRUPTION   = '9', /* 波动性中断 */
    FB_INQUIRY                 = 'A', /* 询价中 */
    FB_CLOSED_AUCTION_ORDERING = 'B', /* 收盘集合竞价 */
    FB_AFTER_TRADING           = 'C'  /* 盘后交易 */
};

}  // namespace api
}  // namespace fb
}  // namespace cffex

#endif
