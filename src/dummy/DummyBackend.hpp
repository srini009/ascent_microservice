/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __DUMMY_BACKEND_HPP
#define __DUMMY_BACKEND_HPP

#include <ams/Backend.hpp>
#include <ascent/ascent.hpp>
#include <queue> 

using json = nlohmann::json;

/**
 * Dummy implementation of an ams Backend.
 */

class ConduitNodeData {

    public:

    conduit::Node m_data;
    conduit::Node m_open_opts;
    conduit::Node m_actions;
    int m_task_id;

    unsigned int m_ts;
    /**
     * @brief Constructor.
     */
    ConduitNodeData(conduit::Node data, conduit::Node open_opts, conduit::Node actions, unsigned int ts, int task_id)
    : m_data(data), 
      m_open_opts(open_opts),
      m_actions(actions),
      m_ts(ts),
      m_task_id(task_id) {}

    /**
     * @brief Move-constructor.
     */
    ConduitNodeData(ConduitNodeData&&) = default;

    /**
     * @brief Copy-constructor.
     */
    ConduitNodeData(const ConduitNodeData&) = default;

    /**
     * @brief Move-assignment operator.
     */
    ConduitNodeData& operator=(ConduitNodeData&&) = default;

    /**
     * @brief Copy-assignment operator.
     */
    ConduitNodeData& operator=(const ConduitNodeData&) = default;

    /**
     * @brief Destructor.
     */
    virtual ~ConduitNodeData() = default;
};

class Compare
{
public:
    bool operator() (ConduitNodeData const& a, ConduitNodeData const& b)
    {
        if(a.m_ts > b.m_ts)
   	    return true;
	else
	    return false;
    }
};


class DummyNode : public ams::Backend {
   
    json m_config;
    ascent::Ascent ascent_lib;
    std::priority_queue <ConduitNodeData, std::vector<ConduitNodeData>, Compare> pq;

    public:

    // SYMBIOMON metrics
    symbiomon_provider_t m_metric_provider;
    uint8_t m_metric_provider_id;
    symbiomon_metric_t m_server_state;
    symbiomon_taglist_t m_taglist;

    /**
     * @brief Constructor.
     */
    DummyNode(const json& config, const thallium::engine& engine)
    : m_config(config) {

        /* Bootstrap to create SYMBIOMON metrics */
        struct symbiomon_provider_args args = SYMBIOMON_PROVIDER_ARGS_INIT;
        args.push_finalize_callback = 0;

        int ret = symbiomon_provider_register(engine.get_margo_instance(), 42, &args, &m_metric_provider);
        if(ret != 0)
            fprintf(stderr, "Error: symbiomon_provider_register() failed. Continuing on.\n");

        m_metric_provider_id = 42;
        symbiomon_taglist_create(&m_taglist, 1, "dummytag");

        if(m_metric_provider != NULL) {
            symbiomon_metric_create("ams", "server_state", SYMBIOMON_TYPE_GAUGE, "ams:server_state", m_taglist, &m_server_state, m_metric_provider);
            fprintf(stderr, "Metric created successfully!!\n");
        }
    }

    /**
     * @brief Constructor.
     */
    DummyNode(const json& config)
    : m_config(config) {
    }

    /**
     * @brief Move-constructor.
     */
    DummyNode(DummyNode&&) = default;

    /**
     * @brief Copy-constructor.
     */
    DummyNode(const DummyNode&) = default;

    /**
     * @brief Move-assignment operator.
     */
    DummyNode& operator=(DummyNode&&) = default;

    /**
     * @brief Copy-assignment operator.
     */
    DummyNode& operator=(const DummyNode&) = default;

    /**
     * @brief Destructor.
     */
    virtual ~DummyNode() = default;

    /**
     * @brief Prints Hello World.
     */
    void sayHello() override;

    /**
     * @brief Executes pending requests
     */
    void ams_execute_pending_requests(size_t pool_size, MPI_Comm comm) override;

    /* Helper function that executes one request */
    void ams_execute_one_request(MPI_Comm comm, ascent::Ascent& a_lib, int rank, int size);

    /**
     * @brief Opens Ascent with a given set of actions.
     */
    ams::RequestResult<bool> ams_open(std::string opts) override;

    /**
     * @brief Closes Ascent.
     */
    ams::RequestResult<bool> ams_close() override;

    /**
     * @brief Publishes a mesh to Ascent.
     */
    ams::RequestResult<bool> ams_publish(std::string bp_mesh) override;

    /**
     * @brief Executes a set of actions in Ascent.
     */
    ams::RequestResult<bool> ams_execute(std::string actions) override;

    /**
     * @brief Publishes a mesh and executes a set of actions in Ascent.
     */
    ams::RequestResult<bool> ams_publish_and_execute(std::string bp_mesh, std::string actions) override;

    /**
     * @brief Publishes a mesh and executes a set of actions in Ascent.
     */
    ams::RequestResult<bool> ams_open_publish_execute(std::string open_opts, std::string bp_mesh, size_t mesh_size, std::string actions, unsigned int ts, size_t pool_size, MPI_Comm comm) override;

    /**
     * @brief Compute the sum of two integers.
     *
     * @param x first integer
     * @param y second integer
     *
     * @return a RequestResult containing the result.
     */
    ams::RequestResult<int32_t> computeSum(int32_t x, int32_t y) override;

    /**
     * @brief Destroys the underlying node.
     *
     * @return a RequestResult<bool> instance indicating
     * whether the database was successfully destroyed.
     */
    ams::RequestResult<bool> destroy() override;

    /**
     * @brief Static factory function used by the NodeFactory to
     * create a DummyNode.
     *
     * @param engine Thallium engine
     * @param config JSON configuration for the node
     *
     * @return a unique_ptr to a node
     */
    static std::unique_ptr<ams::Backend> create(const thallium::engine& engine, const json& config);

    /**
     * @brief Static factory function used by the NodeFactory to
     * open a DummyNode.
     *
     * @param engine Thallium engine
     * @param config JSON configuration for the node
     *
     * @return a unique_ptr to a node
     */
    static std::unique_ptr<ams::Backend> open(const thallium::engine& engine, const json& config);
};

#endif
