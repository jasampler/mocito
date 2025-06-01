# Mocito - Mocking library for writing unit tests of C code like Mockito for Java

Mocito helps you to write unit tests in C in a similar way and spirit than the popular Java mocking library Mockito.

The purpose of Mocito is to test code that uses certain external functions that must not be called when running unit tests, by replacing them with other functions controlled by the programmer, called mocks, that will be configured for responding in the way needed for each unit test.

Suppose that you are testing a code that reads from a database, but you don't want to create a database just for tests. A solution is to replace the function that connects to the database with a mock, and then configure that mock in the unit tests to get different responses from the function. You can do that with Mocito.

To replace a function with a mock, in the mock you must call to `moc_act()` for sending to Mocito: the name of the function, its return type and the received parameters, and then wait for the response, which includes the value to be returned, for example:

    #include <mysql.h>
    #include "mocito.h"
    int mysql_query(MYSQL *mysql, const char *stmt_str) {
        return moc_get_i(moc_act("mysql_query", moc_type_i(),
                moc_values_2(moc_p(mysql), moc_cp_c(stmt_str))));
    }

To configure the created mocks in the unit tests, if you already initialized the memory for the configurations by calling to `moc_init()`, you can call to `moc_given()` for controlling the response of a mock by sending to Mocito: the name of the function, a group of "matchers" and a group of "responders", and after that you can execute the code that calls to the mocked function and then verify the expected behaviour of the executed code, for example:
    
    #include <stdio.h>
    #include <stdlib.h>
    #include <assert.h>
    #include "mocito.h"

    void moc_error(void) { fprintf(stderr, "%s\n", moc_errmsg()); exit(1); }

    void test_get_client_name_if_mysql_query_fails(void) {
        char mem[2500];
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
        n = 0;
        result = get_client_name(&con, 333);

        /* 3. Then: Verify the behaviour of the executed code */
         assert(result == NULL);
         assert(n == 1L);
    }

When Mocito receives a call to a mock, it will search, in the configurations of that function, a group of "matchers" that match with the current parameters and, if a group like that is found, it will execute its group of "responders" associated, returning the value returned by the last responder. If not found, Mocito will call to your function `moc_error()` to report that you forgot to configure correctly the called mock. In the previous example, we included an additional counter to verify that the configured mock was called.

## Features

  - ANSI-C for maximum compatibility with legacy code and systems.
  - Zero dependencies even from a C standard library for more portability.
  - Consistent use of the `moc_` prefix to avoid collisions with other code.
  - Easy to use syntax for writing mocked functions and given-when-then tests.
  - Support for any number of matchers and responders in each configuration.
  - Detection of unexpected types in the mock arguments and returned values.
  - Predefined equality/inequality matchers for the basic C types and strings.
  - Predefined matchers accepting any value for an argument, optionally with type.
  - Predefined responders for returning a value of the basic C types.
  - Predefined responders for counting the number of calls to a mocked function.
  - Support for alternated responses in multiple calls to a mocked function.
  - Support for the creation of user-defined matchers and responders.


