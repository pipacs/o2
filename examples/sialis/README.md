# Sialis

GUI demo for O2, demonstrating using O2 from QML, authenticating with Twitter and sending signed requests in order to show some tweets.

To try out Sialis:

1. Login to developer.twitter.com with your Twitter credentials
1. Create a new application "Sialis", record its API Key and API Secret
   * Enable OAuth 1.0a authentication
   * Set the callback URI to _http://127.0.0.1:8888/_
1. In _main.qml_, change the O1Twitter object's _clientId_ property to the application's API Key
1. Change the _clientSecret_ property to the application's API secret
1. Build and run, press the _Login_ button to log in to Twitter and show the latest tweets
