# OAuth 1.0 and 2.0 for Qt

This library encapsulates the OAuth 1.0 and 2.0 client authentication flows, and the sending of authenticated HTTP requests.

The primary target is Qt Quick applications on mobile devices.

## Contents

Class | Header | Purpose
:-- | :-- | :--
O1 | o1.h | Generic OAuth 1.0 authentication
O1RequestParameter | o1.h | An extra request parameter participating in request signing
O1Dropbox | o1dropbox.h | Dropbox OAuth specializations
O1Flickr | o1flickr.h | Flickr OAuth specializations
O1Requestor | o1requestor.h | Makes authenticated OAuth 1.0 requests: GET, POST or PUT, handles timeouts
O1Twitter | o1twitter.h | Twitter OAuth specializations
OXTwitter | oxtwitter.h | Twitter XAuth specialization
O2 | o2.h | Generic OAuth 2.0 authentication
O2Facebook | o2facebook.h | Facebook OAuth specialization
O2Gft | o2gft.h | Google Fusion Tables OAuth specialization
O2Reply | o2reply.h | A network request/reply that can time out
O2ReplyServer | o2replyserver.h | HTTP server to process authentication responses
O2Requestor | o2requestor.h | Makes authenticated OAuth 2.0 requests (GET, POST or PUT), handles timeouts and token expiry
O2Skydrive | o2skydrive.h | Skydrive OAuth specialization
SimpleCrypt | simplecrypt.h | Simple encryption and decryption by Andre Somers
O2AbstractStore | o2abstractstore.h | Base class for implemnting persistent stores
O2SettingsStore | o2settingsstore.h | A QSettings based persistent store for writing OAuth tokens

## Installation

Clone the Github repository, then add all files to your Qt project.

## Basic Usage

This example assumes a hypothetical Twitter client that will post tweets. Twitter is using OAuth 1.0.

### Setup

Include the required header files, and have some member variables that will be used for authentication and sending requests:

    #include "o1twitter.h"
    #include "o1requestor.h"
    O1Twitter *o1;

### Initialization

Instantiate one of the authenticator classes, like O1Twitter, set your application ID and application secret, and install the signal handlers:

    o1 = new O1Twitter(this);
    o1->setClientId(MY_CLIENT_ID);
    o1->setClientSecret(MY_CLIENT_SECRET);
    connect(o1, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
    connect(o1, SIGNAL(linkingFailed()), this, SLOT(onLinkingFailed()));
    connect(o1, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
    connect(o1, SIGNAL(openBrowser(QUrl)), this, SLOT(onOpenBrowser(QUrl)));
    connect(o1, SIGNAL(closeBrowser()), this, SLOT(onCloseBrowser()));

**Note:** For browserless Twitter authentication, you can use the OXTwitter specialized class which can do Twitter XAuth. You will need to additionally provide your Twitter login credentials (username & password) before calling *link()*.

### Handling Signals

O2 is an asynchronous library. It will send signals at various stages of authentication and request processing.

To handle these signals, implement the following slots in your code:

    void onLinkedChanged() {
        // Linking (login) state has changed.
        // Use o1->linked() to get the actual state
    }

    void onLinkingFailed() {
        // Login has failed
    }

    void onLinkingSucceeded() {
        // Login has succeeded
    }

    void onOpenBrowser(const QUrl *url) {
        // Open a web browser or a web view with the given URL.
        // The user will interact with this browser window to
        // enter login name, password, and authorize your application
        // to access the Twitter account
    }

    void onCloseBrowser() {
        // Close the browser window opened in openBrowser()
    }

### Logging In

To log in (or, to be more accurate, to link your application to the OAuth service), call the link() method:

    o1->link();

This initiates the authentication sequence. Your signal handlers above will be called at various stages. Lastly, if linking succeeds, onLinkingSucceeded() will be called.

### Logging Out

To log out, call the unlink() method:

    o1->unlink();

Logging out always succeeds, and requires no user interaction.

### Sending Authenticated Requests

Once linked, you can start sending authenticated requests to the service. We start with a simple example of sending a text-only tweet or as it's known in Twitter docs, a 'status update'.

First we need a Qt network manager and an O1 requestor object:

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    O1Requestor* requestor = new O1Requestor(manager, o1, this);

Next, create parameters for posting the update:

    QByteArray paramName("status");
    QByteArray tweetText("My first tweet!");

    QList<O1RequestParameter> reqParams = QList<O1RequestParameter>();
    reqParams << O1RequestParameter(paramName, tweetText);

    QByteArray postData = O1::createQueryParams(reqParams);

    // Using Twitter's REST API ver 1.1
    QUrl url = QUrl("https://api.twitter.com/1.1/statuses/update.json");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);

Finally we authenticate and send the request using the O1 requestor object:

    QNetworkReply *reply = requestor->post(request, reqParams, postData);

Continuing with the example, we will now send a tweet containing an image as well as a message.

We create an HTTP request containing the image and the message, in the format specified by Twitter:

    QString imagePath("/tmp/image.jpg");
    QString message("My tweet with an image!");

    QFileInfo fileInfo(imagePath);
    QFile file(imagePath);
    file.open(QIODevice::ReadOnly);

    QString boundary("7d44e178b0439");
    QByteArray data(QString("--" + boundary + "\r\n").toAscii());
    data += "Content-Disposition: form-data; name=\"media[]\"; filename=\"" + fileInfo.fileName() + "\"\r\n";
    data += "Content-Transfer-Encoding: binary\r\n";
    data += "Content-Type: application/octet-stream\r\n\r\n";
    data += file.readAll();
    file.close();
    data += QString("\r\n--") + boundary + "\r\n";
    data += "Content-Disposition: form-data; name=\"status\"\r\n";
    data += "Content-Transfer-Encoding: binary\r\n";
    data += "Content-Type: text/plain; charset=utf-8\r\n\r\n";
    data += message.toUtf8();
    data += QString("\r\n--") + boundary + "--\r\n";

    QNetworkRequest request;
    // Using Twitter's REST API ver 1.1
    static const char *uploadUrl = "https://api.twitter.com/1.1/statuses/update_with_media.json";
    request.setUrl(QUrl(uploadUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.length());

    QNetworkReply *reply = requestor->post(request, QList<O1RequestParameter>(), data);

That's it. Tweets using the O2 library!

### Storing OAuth Tokens

O2 provides simple storage classes for writing OAuth tokens in a peristent location. Currently, a QSettings based backing store **O2SettingsStore** is provided in O2. O2SettingsStore keeps all token values in an encrypted form. You have to specify the encryption key to use while constructing the object:

    O2SettingsStore settings = new O2SettingsStore("myencryptionkey");
    // Set the store before starting OAuth, i.e before calling link()
    o1->setStore(settings);
    ...

You can also create it with your customized QSettings object. O2SettingsStore will then use that QSettings object for storing the tokens:

    O2SettingsStore settings = new O2SettingsStore(mySettingsObject, "myencryptionkey");

Once set, O2SettingsStore takes ownership of the QSettings object.

**Note:** If you do not specify a storage object to use, O2 will create one by default (which QSettings based), and use it. In such a case, a default encryption key is used for encrypting the data.

### Extra OAuth Tokens
Some OAuth service providers provide additional information in the access token response. Eg: Twitter returns 2 additional tokens in it's access token response - *screen_name* and *user_id*.

O2 provides all such tokens via the property - *extraTokens*. You can query this property after a successful OAuth exchange, i.e after the *linkingSucceeded()* signal has been emitted.

