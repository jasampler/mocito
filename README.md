# Mocito - Mocking library for writing unit tests of C code like Mockito for Java

Mocito helps you to write unit tests in C in a similar way and spirit than the popular Java mocking library Mockito.

The purpose of Mocito is to test code that uses certain external functions that must not be called when running unit tests, by replacing them with other functions controlled by the programmer, called mocks, that will be configured for responding in the way needed for each unit test.

For example, if you want to replace a function that reads from a database, and then for a unit test of your code you want to simulate that the function fails, you can do it with Mocito with this code:

    #include <mocito.h>
    ...
    int mysql_query(MYSQL *mysql, const char *stmt_str) {
      return moc_get_i(moc_act("mysql_query", moc_type_i(),
          moc_values_2(moc_p(mysql), moc_cp_c(stmt_str))));
    }
    ...
    #include <assert.h>
    char mem[2500];
    void moc_error(void) { fprintf(stderr, "%s\n", moc_errmsg()); exit(1); }
    ...
    void test_get_client_id_if_mysql_query_fails(void) {
      MYSQL con;
      const char *result;
      moc_init(mem, sizeof(mem));

      /* 1. Given: Configure the responses of the mocks */
      moc_given("mysql_query",
          moc_match_2(moc_eq(moc_p(&con)),
              moc_cstr_eq("SELECT name FROM clients WHERE id=333")),
          moc_respond_1(moc_return(moc_i(-1))));

      /* 2. When: Execute the code that calls the mocks */
      result = get_client_name(&con, 333);

      /* 3. Then: Verify the response of the code */
      assert(result == NULL);
    }

In the first use of Mocito, for writing the mock function, you just send to Mocito the name of the function, its return type and the received parameters by using moc_act(), and then wait for a response. In the second use of mocito, after initializing the memory of the mocks, you send to Mocito the configuration of the mocks by using moc_given(), which gets the name of a function, a group of "matchers" and a group of "responders".

When Mocito receives a call to a mock, it will search, in the configurations of that function, a group of "matchers" that match with the current parameters and, if it is found, then it will execute the group of "responders" associated, returning the value returned by the last responder. If not found, then Mocito will call to our function moc_error() to report that you forgot to configure correctly the called mock.
