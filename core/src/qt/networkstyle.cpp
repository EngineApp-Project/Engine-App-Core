/***********************************************************************
**************Copyright (c) 2014 The Bitcoin developers*****************
*************Copyright (c) 2017-2019 The PIVX developers****************
******************Copyright (c) 2010-2023 Nur1Labs**********************
>Distributed under the MIT software license, see the accompanying
>file COPYING or http://www.opensource.org/licenses/mit-license.php.
************************************************************************/

#include "networkstyle.h"

#include "guiconstants.h"

#include <QApplication>

static const struct {
    const char* networkId;
    const char* appName;
    const char* appIcon;
    const char* titleAddText;
    const char* splashImage;
} network_styles[] = {
    //Gold For Deploy May 31,2017
    {"lilith", QAPP_APP_NAME_DEFAULT, ":/icons/mubdi", QT_TRANSLATE_NOOP("SplashScreen", "[Gold Network Base]"), ":/bg-splash-png"},
    {"test", QAPP_APP_NAME_TESTNET, ":/icons/mubdi_testnet", QT_TRANSLATE_NOOP("SplashScreen", "[testnet]"), ":/bg-splash-png"},
    {"regtest", QAPP_APP_NAME_TESTNET, ":/icons/mubdi_testnet", "[regtest]", ":/bg-splash-png"}};
static const unsigned network_styles_count = sizeof(network_styles) / sizeof(*network_styles);

// titleAddText needs to be const char* for tr()
NetworkStyle::NetworkStyle(const QString& appName, const QString& appIcon, const char* titleAddText, const QString& splashImage) : appName(appName),
                                                                                                                                   appIcon(appIcon),
                                                                                                                                   titleAddText(qApp->translate("SplashScreen", titleAddText)),
                                                                                                                                   splashImage(splashImage)
{
}

const NetworkStyle* NetworkStyle::instantiate(const QString& networkId)
{
    for (unsigned x = 0; x < network_styles_count; ++x) {
        if (networkId == network_styles[x].networkId) {
            return new NetworkStyle(
                network_styles[x].appName,
                network_styles[x].appIcon,
                network_styles[x].titleAddText,
                network_styles[x].splashImage);
        }
    }
    return 0;
}
