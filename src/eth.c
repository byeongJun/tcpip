#include "eth.h"

#include "hw.h"

#include <memory.h>

typedef union PACKED eth_frame
{
    struct PACKED
    {
        uint8_t  dst_addr[ETH_ADDR_LEN];
        uint8_t  src_addr[ETH_ADDR_LEN];
        uint16_t ether_type;
        uint8_t  data[ETH_DATA_MAX_LEN];
    };

    uint8_t buffer[ETH_FRAME_MAX_LEN];

} eth_frame_t;

size_t eth_send(session_t *session, const uint8_t dst_addr[],
                const uint8_t data[], size_t data_len)
{
    // We can send maximum of ETH_DATA_MAX_LEN bytes of data.
    if(data_len > ETH_DATA_MAX_LEN)
        return 0;

    eth_frame_t frame;

    // Set the header
    memcpy(frame.dst_addr, dst_addr, ETH_ADDR_LEN);
    memcpy(frame.src_addr, session->src_addr, ETH_ADDR_LEN);
    frame.ether_type = netb_s(ETH_PROTOCOL_IPV6);

    // Set the data.
    memset(frame.data, 0, ETH_DATA_MIN_LEN);
    memcpy(frame.data, data, data_len);

    // Send the frame. We can send minimum of ETH_FRAME_MIN_LEN bytes.
    const size_t frame_len = MAX(ETH_FRAME_MIN_LEN, ETH_HEADER_LEN + data_len);
    const size_t sent = hw_send(session->session_id, frame.buffer, frame_len);

    return sent == frame_len ? data_len : 0;
}

size_t eth_recv(session_t *session, uint8_t data[])
{
    eth_frame_t frame;

    const size_t frame_len = hw_recv(session->session_id, session->recv_timeout,
                                     frame.buffer, sizeof(frame.buffer));

    if(frame_len == (size_t)-1)
        return 0;

    const size_t data_len = frame_len - ETH_HEADER_LEN;
    memcpy(data, frame.data, data_len);

    return data_len;
}
