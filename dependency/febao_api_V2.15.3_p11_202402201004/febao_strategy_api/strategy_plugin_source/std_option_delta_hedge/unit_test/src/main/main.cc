/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: zhr
 * Date: 2018-03-05
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stdexcept>
#include <gtest/gtest.h>
#include <stdarg.h>

using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;


void sig_ignore(int n) {
    //fprintf(stdout, "%s, signal[%d]\n", __FUNCTION__, n);
}
void sig_exit(int n) {
    fprintf(stdout, "%s, signal[%d]\n", __FUNCTION__, n);
    exit(0);
}

int main(int argc, char **argv)
{
    //cffex::signal(SIGPIPE, sig_ignore);
    //cffex::signal(SIGINT,  sig_exit);
    //cffex::signal(SIGBUS, sig_ignore);

    int ret  =  0;
    try {
        InitGoogleTest(&argc, argv);
        ret = RUN_ALL_TESTS();
    } catch(std::runtime_error e) {
        fprintf(stdout, "exception[%s]\n", e.what());
    }

#ifdef WIN32
    system("pause");
#endif

    return ret;

}



