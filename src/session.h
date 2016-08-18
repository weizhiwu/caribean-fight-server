#ifndef _SESSION_H_
#define _SESSION_H_

#include <stddef.h>
#include <stdint.h>
#include <string>
#include "action.h"
#include "ring_queue.h"
#include "udp_pkg_def.h"
#include "common_def.h"
#include "room.h"
#include "room_mgr.h"

class Frame;
class FrameMgr;

typedef struct SeqInfo
{
    SeqInfo() : seq_id(0), max_frame_id(0), total_frame_store(0), used(0)
    {}

    SeqInfo(int seq_id, int max_frame_id, int total_frame_store, int used)
            : seq_id(seq_id), max_frame_id(max_frame_id), total_frame_store(total_frame_store), used(used)
    {}

    int seq_id;
    int max_frame_id;
    int total_frame_store;
    size_t used;
    int id() const { return seq_id; }
}SeqInfo;

/*Every TCP conn is a session, which stand for a player*/
class Session
{
public:
    Session(boost::asio::io_service& io_service)
            : m_socket(io_service), m_rid(0), m_sid("")
    {
        m_socket_ptr = SocketPtr(&m_socket);
    }

    tcp::socket& Socket()
    {
        return m_socket;
    }

    void Start()
    {
        m_socket.async_read_some(boost::asio::buffer(m_pkg, MAX_PKG_SIZE),
                                boost::bind(&Session::HandleRead, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void HandleRead(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if (!error)
        {
            cout<<"Trace: recvd: " << std::string(m_pkg) <<std::endl;
            std::string rsp = this->HandlePkg(std::string(m_pkg));
            cout<<"Trace: rsp: " <<rsp <<std::endl;

            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(rsp, rsp.length()),
                                     boost::bind(&Session::HandleWrite, this,
                                                 boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }

    void HandleWrite(const boost::system::error_code& error)
    {
        if (!error)
        {
            m_socket.async_read_some(boost::asio::buffer(m_pkg, MAX_PKG_SIZE),
                                    boost::bind(&Session::HandleRead, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            delete this;
        }
    }

public:
    string HandlePkg(std::string pkg);

    void SetId(std::string id) { m_sid = id; }
    std::string GetId() const { return m_sid; }
    unsigned GetRId() const {return m_rid; }
    SocketPtr GetSocketPtr() { return m_socket_ptr; }

private:
    //they are the same thing
    tcp::socket m_socket;
    SocketPtr m_socket_ptr;

    unsigned m_rid;    //room id
    enum { MAX_PKG_SIZE = 1024 };
    char m_pkg[MAX_PKG_SIZE];

    std::string m_sid; //session id
};

#endif
