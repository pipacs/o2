cmake_minimum_required(VERSION 3.16)

if(o2_WITH_FACEBOOK)
    add_subdirectory(facebookdemo)
    add_subdirectory(facebookexternalinterceptordemo)
endif(o2_WITH_FACEBOOK)

if(o2_WITH_GOOGLE)
    add_subdirectory(youtubedemo)
endif(o2_WITH_GOOGLE)

if(o2_WITH_TWITTER)
    add_subdirectory(twitterdemo)
endif(o2_WITH_TWITTER)

if (o2_WITH_VIMEO)
    add_subdirectory(vimeodemo)
endif(o2_WITH_VIMEO)

if (o2_WITH_YOUTUBE)
    add_subdirectory(youtubedemo)
endif(o2_WITH_YOUTUBE)

if (o2_WITH_MSGRAPH)
    add_subdirectory(msgraphdemo)
    add_subdirectory(msgraphexternalinterceptordemo)
endif(o2_WITH_MSGRAPH)
