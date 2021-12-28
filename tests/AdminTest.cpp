/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <ams/Admin.hpp>
#include <cppunit/extensions/HelperMacros.h>

extern thallium::engine engine;
extern std::string node_type;

class AdminTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( AdminTest );
    CPPUNIT_TEST( testAdminCreateNode );
    CPPUNIT_TEST_SUITE_END();

    static constexpr const char* node_config = "{ \"path\" : \"mydb\" }";

    public:

    void setUp() {}
    void tearDown() {}

    void testAdminCreateNode() {
        ams::Admin admin(engine);
        std::string addr = engine.self();

        ams::UUID node_id;
        // Create a valid Node
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("admin.createNode should return a valid Node",
                node_id = admin.createNode(addr, 0, node_type, node_config));

        // Create a Node with a wrong backend type
        ams::UUID bad_id;
        CPPUNIT_ASSERT_THROW_MESSAGE("admin.createNode should throw an exception (wrong backend)",
                bad_id = admin.createNode(addr, 0, "blabla", node_config),
                ams::Exception);

        // Destroy the Node
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("admin.destroyNode should not throw on valid Node",
            admin.destroyNode(addr, 0, node_id));

        // Destroy an invalid Node
        CPPUNIT_ASSERT_THROW_MESSAGE("admin.destroyNode should throw on invalid Node",
            admin.destroyNode(addr, 0, bad_id),
            ams::Exception);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION( AdminTest );
