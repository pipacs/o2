cmake_minimum_required(VERSION 2.8.11)

if(WITH_FACEBOOK)
    add_subdirectory(facebookdemo)
endif(WITH_FACEBOOK)

if(WITH_TWITTER)
    add_subdirectory(twitterdemo)
    add_subdirectory(sialis)
endif(WITH_TWITTER)
