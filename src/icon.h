#pragma once

static QIcon iconFromSetting(const QString &setting) {
    if(setting.startsWith("/")) { // filename
        return QIcon(setting);
    } else {
        qDebug() << setting;
        return QIcon::fromTheme(setting);
    }
}
