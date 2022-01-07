/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AMS_CLIENT_IMPL_H
#define __AMS_CLIENT_IMPL_H

#include <thallium.hpp>
#include <thallium/serialization/stl/unordered_set.hpp>
#include <thallium/serialization/stl/unordered_map.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <ascent.hpp>

namespace ams {

namespace tl = thallium;

class ClientImpl {

    public:

    tl::engine           m_engine;
    tl::remote_procedure m_check_node;
    tl::remote_procedure m_say_hello;
    tl::remote_procedure m_compute_sum;
    /* SR: Core Ascent APIs */
    tl::remote_procedure m_ams_open;
    tl::remote_procedure m_ams_close;
    tl::remote_procedure m_ams_publish;
    tl::remote_procedure m_ams_execute;
    tl::remote_procedure m_ams_publish_and_execute;
    tl::remote_procedure m_ams_open_publish_execute;

    ClientImpl(const tl::engine& engine)
    : m_engine(engine)
    , m_check_node(m_engine.define("ams_check_node"))
    , m_say_hello(m_engine.define("ams_say_hello").disable_response())
    , m_compute_sum(m_engine.define("ams_compute_sum"))
    , m_ams_open(m_engine.define("ams_open"))
    , m_ams_close(m_engine.define("ams_close"))
    , m_ams_publish(m_engine.define("ams_publish"))
    , m_ams_execute(m_engine.define("ams_execute"))
    , m_ams_publish_and_execute(m_engine.define("ams_publish_and_execute"))
    , m_ams_open_publish_execute(m_engine.define("ams_open_publish_execute"))
    {}

    ClientImpl(margo_instance_id mid)
    : ClientImpl(tl::engine(mid)) {}

    ~ClientImpl() {}
};

}

#endif
