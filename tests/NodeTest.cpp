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

class NodeTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( NodeTest );
    CPPUNIT_TEST( testMakeNodeHandle );
    CPPUNIT_TEST( testSayHello );
    CPPUNIT_TEST( testComputeSum );
    CPPUNIT_TEST_SUITE_END();

    static constexpr const char* node_config = "{ \"path\" : \"mydb\" }";
    ams::UUID node_id;

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

    void testMakeNodeHandle() {
        ams::Client client(engine);
        std::string addr = engine.self();

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "client.makeNodeHandle should not throw for valid id.",
                client.makeNodeHandle(addr, 0, node_id));

        auto bad_id = ams::UUID::generate();
        CPPUNIT_ASSERT_THROW_MESSAGE(
                "client.makeNodeHandle should throw for invalid id.",
                client.makeNodeHandle(addr, 0, bad_id),
                ams::Exception);
        
        CPPUNIT_ASSERT_THROW_MESSAGE(
                "client.makeNodeHandle should throw for invalid provider.",
                client.makeNodeHandle(addr, 1, node_id),
                std::exception);
        
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "client.makeNodeHandle should not throw for invalid id when check=false.",
                client.makeNodeHandle(addr, 0, bad_id, false));

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "client.makeNodeHandle should not throw for invalid provider when check=false.",
                client.makeNodeHandle(addr, 1, node_id, false));
    }

    void testSayHello() {
        ams::Client client(engine);
        std::string addr = engine.self();
        
        ams::NodeHandle my_node = client.makeNodeHandle(addr, 0, node_id);

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_node.sayHello() should not throw.",
                my_node.sayHello());
    }

    void testComputeSum() {
        ams::Client client(engine);
        std::string addr = engine.self();
        
        ams::NodeHandle my_node = client.makeNodeHandle(addr, 0, node_id);

        int32_t result = 0;
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_node.computeSum() should not throw.",
                my_node.computeSum(42, 51, &result));

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "42 + 51 should be 93",
                93, result);

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_node.computeSum() should not throw when passed NULL.",
                my_node.computeSum(42, 51, nullptr));

        ams::AsyncRequest request;
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_node.computeSum() should not throw when called asynchronously.",
                my_node.computeSum(42, 51, &result, &request));

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "request.wait() should not throw.",
                request.wait());
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION( NodeTest );
