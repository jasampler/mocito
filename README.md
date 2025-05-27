# Mocito - Mocking library for writing unit tests of C code like Mockito for Java

Mocito helps you to write unit tests in C in a similar way and spirit than the popular Java mocking library Mockito.

The purpose of Mocito is to test code that uses certain external functions that must not be called when running unit tests, by replacing them with other functions controlled by the programmer, called mocks, that will be configured for responding in the way needed for each unit test.

Suppose that you are testing a code that reads from a database, but you don't want to create a database just for tests. A solution is to replace the function that connects to the database with a mock, and then configure that mock in the unit tests to get different responses from the function. You can do that with Mocito.

To replace a function with a mock, you must send to Mocito the name of the function, its return type and the received parameters calling to moc_act(), and then wait for the response, for example:

    #include <mysql.h>
    #include "mocito.h"
    int mysql_query(MYSQL *mysql, const char *stmt_str) {
        return moc_get_i(moc_act("mysql_query", moc_type_i(),
                moc_values_2(moc_p(mysql), moc_cp_c(stmt_str))));
    }

To configure the mock in the unit tests, after initializing the memory for the mocks, you must send to Mocito the name of the function, a group of "matchers" and a group of "responders" calling to moc_given(), and then execute the code that calls to the mock, for example:
    
    #include <stdio.h>
    #include <stdlib.h>
    #include <assert.h>
    #include "mocito.h"

    char mem[2500];
    void moc_error(void) { fprintf(stderr, "%s\n", moc_errmsg()); exit(1); }

    void test_get_client_name_if_mysql_query_fails(void) {
        MYSQL con;
        const char *result;
        long n;

        moc_init(mem, sizeof(mem));

        /* 1. Given: Configure the responses of the mocks */
        moc_given("mysql_query",
                moc_match_2(moc_eq(moc_p(&con)),
                        moc_eq_cstr("SELECT name FROM clients WHERE id=333")),
                moc_respond_2(moc_count(moc_p_l(&n)),
                        moc_return(moc_i(-1))));

        /* 2. When: Execute the code that calls to the mocks */
        result = get_client_name(&con, 333);

        /* 3. Then: Verify the response of the code */
         assert(result == NULL);
         assert(n == 1L);
    }

When Mocito receives a call to a mock, it will search, in the configurations of that function, a group of "matchers" that match with the current parameters and, if a group like that is found, it will execute its group of "responders" associated, returning the value returned by the last responder. If not found, Mocito will call to your function moc_error() to report that you forgot to configure correctly the called mock.
