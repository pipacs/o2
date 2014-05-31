cmake_minimum_required(VERSION 2.8.11)

if(WITH_IMGUR)
    add_subdirectory(imgurdemo)
endif(WITH_IMGUR)

if(WITH_FACEBOOK)
    add_subdirectory(facebookdemo)
endif(WITH_FACEBOOK)

if(WITH_TWITTER)
    add_subdirectory(twitterdemo)
endif(WITH_TWITTER)
