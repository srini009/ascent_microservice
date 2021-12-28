/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <ams/Client.hpp>
#include <ams/Admin.hpp>

extern thallium::engine engine;
extern std::string node_type;

class ClientTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( ClientTest );
    CPPUNIT_TEST( testOpenNode );
    CPPUNIT_TEST_SUITE_END();

    static constexpr const char* node_config = "{ \"path\" : \"mydb\" }";
    UUID node_id;

    public:

    void setUp() {
        ams::Admin admin(engine);
        std::string addr = engine.self();
        node_id = admin.createNode(addr, 0, node_type, node_config);
    }

    void tearDown() {
        ams::Admin admin(engine);
        std::string addr = engine.self();
        admin.destroyNode(addr, 0, node_id);
    }

    void testOpenNode() {
        ams::Client client(engine);
        std::string addr = engine.self();
        
        Node my_node = client.open(addr, 0, node_id);
        CPPUNIT_ASSERT_MESSAGE(
                "Node should be valid",
                static_cast<bool>(my_node));

        auto bad_id = UUID::generate();
        CPPUNIT_ASSERT_THROW_MESSAGE(
                "client.open should fail on non-existing node",
                client.open(addr, 0, bad_id);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION( ClientTest );
