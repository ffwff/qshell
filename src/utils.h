#pragma once

static inline QIcon iconFromSetting(const QString &setting) {
    if(setting.startsWith("/")) {
        return QIcon(setting);
    } else {
        return QIcon::fromTheme(setting);
    }
}

static inline int dimFromSetting(QString setting, int parentDim=0) {
    if(setting.endsWith("px")) {
        return setting.replace("px","").toInt();
    } else {
        return parentDim * (setting.toFloat() / 100);
    }
}
