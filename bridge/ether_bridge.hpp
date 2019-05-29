#pragma once

#include <netinet/ether.h>

#include <map>
#include <list>
#include <functional>

#include "socket.hpp"

#include <yaml-cpp/yaml.h>

struct ether_bridge_conf {
  enum { SERVER, CLIENT } mode_;
  std::string key_;
  std::string tap_device_;

  // client
  std::string remote_addr_;
  std::uint16_t remote_port_;

  // server
  std::string bind_addr_;
  std::uint16_t bind_port_;

  ether_bridge_conf(const YAML::Node &node) {
    mode_ = node["mode"].as<std::string>() == "server" ? SERVER : CLIENT;
    key_ = node["key"].as<std::string>();
    tap_device_ = node["tap_dev"].as<std::string>();

    if (mode_ == CLIENT) {
      remote_addr_ = node["remote_addr"].as<std::string>();
      remote_port_ = node["remote_port"].as<std::uint16_t>();
    } else {
      bind_addr_ = node["bind_addr"].as<std::string>();
      bind_port_ = node["bind_port"].as<std::uint16_t>();
    }
  }
};

struct ethernet_address
{
    struct ether_addr addr_;

    ethernet_address(const struct ether_addr &addr) : addr_(addr)
    {}
    bool operator <(const ethernet_address &addr) const
    {
        return memcmp(&addr_, &addr, sizeof(addr_)) < 0;
    }
    bool operator ==(const ethernet_address &addr) const
    {
        return memcmp(&addr_, &addr, sizeof(addr_)) == 0;
    }
    bool operator !=(const ethernet_address &addr) const
    {
        return !(*this == addr);
    }
    bool is_broadcast(void)
    {
        return *this == broad_cast_address;
    }
    static struct ether_addr broadcast;
    static ethernet_address broad_cast_address;
};



template <typename T>
class mac_table
{
public:
    mac_table()
    {}

    void update(const ethernet_address &addr, const T &t)
    {
        ethernet_address_record record = {
            addr,
            time(NULL),
            t
        };

        auto it = table_.find(addr);
        if (it != table_.end()) {
            it->second = record;
        } else {
            table_.emplace(std::make_pair(addr, record));
        }
    }

    bool query(const ethernet_address &addr, T &t)
    {
        auto it = table_.find(addr);
        if (it == table_.end())
        {
            return false;
        }
        else
        {
            t = it->second.t;
            return true;
        }
    }

    void check_timeout()
    {
        time_t now = time(NULL);
        for (auto it = table_.begin(); it != table_.end();)
        {
            if (now - it->second.last_active > timeout)
            {
                it = table_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    struct ethernet_address_record
    {
        ethernet_address addr_;
        time_t last_active;
        T t;
    };



    typedef std::map<ethernet_address, ethernet_address_record> table_map_type;

    class iterator : public std::iterator<std::output_iterator_tag, T, long, const T*, T>
    {
        typename table_map_type::iterator it_;
    public:
        explicit iterator(typename table_map_type::iterator it) : it_(it) {}
        iterator &operator++() {it_++;return *this;}
        iterator operator++(int) {iterator retval = *this; ++(*this); return retval;}
        bool operator==(const iterator &other) const {return it_ == other.it_;}
        bool operator!=(const iterator &other) const {return !(*this == other);}
        const T& operator*() const {return it_->second.t;}
        const ethernet_address &addr() const {return it_->first;}
    };

    iterator begin()
    {
        return iterator(table_.begin());
    }

    iterator end()
    {
        return iterator(table_.end());
    }
protected:
    std::map<ethernet_address, ethernet_address_record> table_;
    std::list<ethernet_address_record *> list_;
    enum {timeout = 600};
};

class ethernet_bridge
{
public:
    typedef std::function<void (const byte_buffer &buffer)> forward_handler_type;

    enum {bridge_mtu = 1500};

    ethernet_bridge()
    {
    }

    void forward(const byte_buffer &buffer, forward_handler_type reply_handler)
    {
        clean_up();

        if (buffer.size() < sizeof(struct ether_header)) {
            return;
        }

        const struct ether_header *head = (const struct ether_header *)(const void *)buffer;

        ethernet_address dst(*(ether_addr *)head->ether_dhost);
        ethernet_address src(*(ether_addr *)head->ether_shost);

        table_.update(src, reply_handler);

        forward_handler_type send_handler;
        if (!dst.is_broadcast())
        {
            if (table_.query(dst, send_handler))
            {
                send_handler(buffer);
            }
        }
        else
        {
            for (auto it = table_.begin(); it != table_.end(); ++it)
            {
                if (it.addr() != src)
                {
                    (*it)(buffer);
                }
            }
        }
    }

    void clean_up()
    {
        table_.check_timeout();
    }
protected:
    mac_table<forward_handler_type> table_;
};
