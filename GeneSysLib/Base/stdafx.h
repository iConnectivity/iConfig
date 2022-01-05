/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <bitset>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#ifndef Q_MOC_RUN
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/array.hpp>
#include <boost/assign.hpp>
#include <boost/bind.hpp>
#include <boost/container/vector.hpp>
#include <boost/format.hpp>
#include <boost/functional.hpp>
#include <boost/move/move.hpp>
#include <boost/optional.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/irange.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/swap.hpp>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#endif

#ifndef __IOS__
#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QHttp>
#include <QLabel>
#include <QList>
#include <QLineEdit>
#include <QMainWindow>
#include <QMap>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QMetaType>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QProgressBar>
#include <QProgressDialog>
#include <QRegExp>
#include <QSet>
#include <QSettings>
#include <QSignalMapper>
#include <QSpacerItem>
#include <QSpinBox>
#include <QString>
#include <QTableWidget>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QWaitCondition>
#endif

#include "LibTypes.h"
#include "StreamHelpers.h"

#ifdef WIN32

void usleep(__int64 usec);

#endif

#endif  // __STDAFX_H__
