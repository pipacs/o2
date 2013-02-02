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
O2 | o2.h | Generic OAuth 2.0 authentication
O2Facebook | o2facebook.h | Facebook OAuth specialization
O2Gft | o2gft.h | Google Fusion Tables OAuth specialization
O2Reply | o2reply.h | A network request/reply that can time out
O2ReplyServer | o2replyserver.h | HTTP server to process authentication responses
O2Requestor | o2requestor.h | Makes authenticated OAuth 2.0 requests (GET, POST or PUT), handles timeouts and token expiry
O2Skydrive | o2skydrive.h | Skydrive OAuth specialization
SimpleCrypt | simplecrypt.h | Simple encryption and decryption by Andre Somers

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

Once linked, you can start sending authenticated requests to the service. Continuing the example use case, we are going to send a tweet to Twitter, containing an image and a message.

First we need a Qt network manager and an O1 requestor object:

    QNetworkAccessManager manager = new QNetworkAccessManager(this);
    O1Requestor requestor = new O1Requestor(manager, o1, this);
    
Then we create an HTTP request containing the image and the message, in the format specified by Twitter:

    QString imagePath("/tmp/image.jpg");
    QString message("My first tweet!");
    
    QFileInfo fileInfo(imagePath);
    QFile file(imagePath);
    
    QString boundary("7d44e178b0439");
    QByteArray data(QString("--" + boundary + "\r\n").toAscii());
    data += "Content-Disposition: form-data; name=\"media[]\"; filename=\"" + fileInfo.baseName() + "\"\r\n";
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
    static const char *uploadUrl = "https://upload.twitter.com/1/statuses/update_with_media.json";
    request.setUrl(QUrl(uploadUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.length());
    
Finally we authenticate and send the request using the O1 requestor object:

    QNetworkReply *reply = requestor->post(request, QList<O1RequestParameter>(), data);

That's it. A tweet using the O2 library!
