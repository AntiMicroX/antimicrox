#ifndef WINSTARTUP_H
#define WINSTARTUP_H

#include <QString>

/** @brief Manage the current user's startup shortcut and elevated logon task. */
class WinStartup
{
  public:
    enum class Mode
    {
        Disabled,
        Normal,
        Administrator
    };

    explicit WinStartup(const QString &executable, const QString &shortcut_path = QString(),
                        const QString &task_name = QString());
    bool read(Mode &mode, QString &error) const;
    bool write(Mode mode, QString &error) const;

  private:
    QString m_executable;
    QString m_shortcut_path;
    QString m_task_name;
    QString m_user_sid;
};

#endif // WINSTARTUP_H
