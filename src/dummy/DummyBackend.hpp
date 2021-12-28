/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __DUMMY_BACKEND_HPP
#define __DUMMY_BACKEND_HPP

#include <ams/Backend.hpp>

using json = nlohmann::json;

/**
 * Dummy implementation of an ams Backend.
 */
class DummyNode : public ams::Backend {
   
    json m_config;

    public:

    /**
     * @brief Constructor.
     */
    DummyNode(const json& config)
    : m_config(config) {}

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
