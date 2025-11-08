#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include "configmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SettingDialog;
}
QT_END_NAMESPACE

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();

private slots:
    void onResetButtonClicked();
    void onBrowseSourceButtonClicked();
    void onBrowseTargetButtonClicked();
    void onThemeChanged();
    void accept() override;

private:
    Ui::SettingDialog *ui;

    void loadSettings();
    void saveSettings();
    void resetToDefaults();
    void connectSignals();
    void applyTheme(const QString &theme);

    TranscodeSettings getTranscodeSettingsFromUI() const;
    SystemSettings getSystemSettingsFromUI() const;
    void setTranscodeSettingsToUI(const TranscodeSettings &settings);
    void setSystemSettingsToUI(const SystemSettings &settings);
};

#endif // SETTINGDIALOG_H
