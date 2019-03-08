/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef GETSETFUNC_H
#define GETSETFUNC_H

#ifndef Q_MOC_RUN
#include <boost/function.hpp>
#endif

class QComboBox;
class QString;
class QStringList;

#ifndef Q_MOC_RUN
typedef boost::function<bool(void)> GetBoolFunctor;
typedef boost::function<void(bool)> SetBoolFunctor;

typedef boost::function<QString(void)> GetQStringFunctor;
typedef boost::function<void(QString)> SetQStringFunctor;

typedef boost::function<int(void)> GetIntFunctor;
typedef boost::function<void(int)> SetIntFunctor;

typedef boost::function<QStringList(void)> GetComboBoxOptions;
typedef boost::function<int(QComboBox *)> GetComboBoxSelectionFunctor;
typedef boost::function<void(QComboBox *, int)> SetComboBoxSelectionFunctor;
#endif

#endif  // GETSETFUNC_H
