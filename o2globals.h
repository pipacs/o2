/*
 *   Copyright 2012 Infosys Technologies Ltd. All rights reserved.
 *   Use of copyright notice does not imply publication.
 *
 *             CONFIDENTIAL INFORMATION
 *             ------------------------
 *   This Document contains Confidential Information or Trade Secrets,
 *   or both, which are the property of Infosys Technologies Ltd.
 *   This document may not be copied, reproduced, reduced to any
 *   electronic medium or machine readable form or otherwise
 *   duplicated and the information herein may not be used,
 *   disseminated or otherwise disclosed except with the prior
 *   written consent of Infosys Technologies Ltd.
 */

#ifndef O2GLOBALS_H
#define O2GLOBALS_H

// Common constants
const char ENC_KEY[]            = "12345678";
const char CALLBACK_URL[]       = "http://127.0.0.1:%1/";
const char MIME_TYPE_XFORM[]    = "application/x-www-form-urlencoded";

// QSettings key names
const char KEY_TOK[]            = "token.%1";
const char KEY_TOK_SECRET[]     = "tokensecret.%1";
const char KEY_CODE[]           = "code.%1";
const char KEY_EXPIRES[]        = "expires.%1";
const char KEY_REFRESH_TOK[]    = "refreshtoken.%1";


// OAuth 1/1.1 Request Params
const char OAUTH_CB[]           = "oauth_callback";
const char OAUTH_CONSUMER_KEY[] = "oauth_consumer_key";
const char OAUTH_NONCE[]        = "oauth_nonce";
const char OAUTH_SIG[]          = "oauth_signature";
const char OAUTH_SIG_METHOD[]   = "oauth_signature_method";
const char OAUTH_TIMESTAMP[]    = "oauth_timestamp";
const char OAUTH_VERSION[]      = "oauth_version";
// OAuth 1/1.1 Response Params
const char OAUTH_TOK[]          = "oauth_token";
const char OAUTH_TOK_SEC[]      = "oauth_token_secret";
const char OAUTH_CB_CONFIRMED[] = "oauth_callback_confirmed";
const char OAUTH_VERFIER[]      = "oauth_verifier";

// OAuth 2 Request Params
const char OAUTH2_RESP_TYPE[]       = "response_type";
const char OAUTH2_CLIENT_ID[]       = "client_id";
const char OAUTH2_CLIENT_SECRET[]   = "client_secret";
const char OAUTH2_REDIRECT_URI[]    = "redirect_uri";
const char OAUTH2_SCOPE[]           = "scope";
const char OAUTH2_CODE[]            = "code";
const char OAUTH2_TOK[]             = "token";
const char OAUTH2_GRANT_TYPE[]      = "grant_type";
// OAuth 2 Response Params
const char OAUTH2_ACCESS_TOK[]      = "access_token";
const char OAUTH2_REFRESH_TOK[]     = "refresh_token";
const char OAUTH2_EXPIRES_IN[]      = "expires_in";

// Parameter values
const char SIG_TYPE_HMAC_SHA1[]     = "HMAC-SHA1";
const char GRANT_TYPE_AUTH_CODE[]   = "authorization_code";

// Std HTTP headres
const char HTTP_AUTH_HEADER[]   = "Authorization";

#endif // O2GLOBALS_H
