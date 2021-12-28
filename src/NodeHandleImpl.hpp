/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AMS_NODE_HANDLE_IMPL_H
#define __AMS_NODE_HANDLE_IMPL_H

#include <ams/UUID.hpp>

namespace ams {

class NodeHandleImpl {

    public:

    UUID                        m_node_id;
    std::shared_ptr<ClientImpl> m_client;
    tl::provider_handle         m_ph;

    NodeHandleImpl() = default;
    
    NodeHandleImpl(const std::shared_ptr<ClientImpl>& client, 
                       tl::provider_handle&& ph,
                       const UUID& node_id)
    : m_node_id(node_id)
    , m_client(client)
    , m_ph(std::move(ph)) {}
};

}

#endif
