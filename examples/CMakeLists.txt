cmake_minimum_required(VERSION 2.8.11)

if(o2_WITH_FACEBOOK)
    add_subdirectory(facebookdemo)
endif(o2_WITH_FACEBOOK)

if(o2_WITH_GOOGLE)
    add_subdirectory(youtubedemo)
endif(o2_WITH_GOOGLE)

if(o2_WITH_TWITTER)
    add_subdirectory(twitterdemo)
endif(o2_WITH_TWITTER)

#if(o2_WITH_SPOTIFY)
#    add_subdirectory(spotifydemo)
#endif(o2_WITH_SPOTIFY)

if (o2_WITH_VIMEO)
    add_subdirectory(vimeodemo)
endif(o2_WITH_VIMEO)

if (o2_WITH_MSGRAPH)
    add_subdirectory(msgraphdemo)
    add_subdirectory(msgraphexternalinterceptordemo)
endif(o2_WITH_MSGRAPH)
