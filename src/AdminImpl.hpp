/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AMS_ADMIN_IMPL_H
#define __AMS_ADMIN_IMPL_H

#include <thallium.hpp>

namespace ams {

namespace tl = thallium;

class AdminImpl {

    public:

    tl::engine           m_engine;
    tl::remote_procedure m_create_node;
    tl::remote_procedure m_open_node;
    tl::remote_procedure m_close_node;
    tl::remote_procedure m_destroy_node;

    AdminImpl(const tl::engine& engine)
    : m_engine(engine)
    , m_create_node(m_engine.define("ams_create_node"))
    , m_open_node(m_engine.define("ams_open_node"))
    , m_close_node(m_engine.define("ams_close_node"))
    , m_destroy_node(m_engine.define("ams_destroy_node"))
    {}

    AdminImpl(margo_instance_id mid)
    : AdminImpl(tl::engine(mid)) {
    }

    ~AdminImpl() {}
};

}

#endif
