/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef X11EXTRAS_H
#define X11EXTRAS_H

#include <QObject>
#include <QPoint>
#include <QHash>

#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>



class X11Extras : public QObject
{
    Q_OBJECT

public:
    struct ptrInformation {
        long id;
        int threshold;
        int accelNum;
        int accelDenom;

        ptrInformation()
        {
            id = -1;
            threshold = 0;
            accelNum = 0;
            accelDenom = 1;
        }
    };

    ~X11Extras();

    unsigned long appRootWindow(int screen = -1);
    Display* display();
    bool hasValidDisplay();
    QString getDisplayString(QString xcodestring);
    int getApplicationPid(Window window);
    QString getApplicationLocation(int pid);
    Window findClientWindow(Window window);
    Window findParentClient(Window window);
    void closeDisplay();
    void syncDisplay();
    void syncDisplay(QString displayString);
    static QString getXDisplayString();
    QString getWindowTitle(Window window);
    QString getWindowClass(Window window);
    unsigned long getWindowInFocus();
    int getGroup1KeySym(int virtualkey);

    void x11ResetMouseAccelerationChange();
    void x11ResetMouseAccelerationChange(QString pointerName);
    struct ptrInformation getPointInformation();
    struct ptrInformation getPointInformation(QString pointerName);

    static void setCustomDisplay(QString displayString);

    const char* getEnvVariable(const char* var) const;

    static X11Extras* getInstance();
    static void deleteInstance();

    QHash<QString, QString> const& getKnownAliases();

protected:
    explicit X11Extras(QObject *parent = nullptr);

    void populateKnownAliases();
    bool windowHasProperty(Display *display, Window window, Atom atom);
    bool windowIsViewable(Display *display, Window window);
    bool isWindowRelevant(Display *display, Window window);

    static X11Extras *_instance;  
    
public slots:
    QPoint getPos();

private:
    void checkPropertyOnWin(bool windowCorrected, Window& window, Window& parent, Window& finalwindow, Window& root, Window *children, Display *display, unsigned int& num_children);
    void freeDisplay();
    void checkFeedback(XFeedbackState *temp, int& num_feedbacks, int& feedback_id);
    void findVirtualPtr(int num_devices, XIDeviceInfo *current_devices, XIDeviceInfo *mouse_device, XIDeviceInfo *all_devices, QString pointerName);

    QHash<QString, QString> knownAliases;
    Display *_display;
};

#endif // X11EXTRAS_H
