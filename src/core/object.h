/*
    MIT License

    Copyright (c) 2025 Evandro

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

#include "config.h"
#include "logger.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <map>
#include <QDebug>
#include <QtCore>
#include <unistd.h>

namespace PCore {
    typedef struct atomic_obj_cpt_t {
        std::atomic<int> constructed;
        std::atomic<int> destructed;
        atomic_obj_cpt_t() : constructed(0), destructed(0) {}
    } atomic_obj_cpt_t;

    typedef struct {
        int constructed;
        int destructed;
    } obj_cpt_t;

    typedef std::map<const char*, obj_cpt_t> object_map_t;
    typedef std::map<const char*, const atomic_obj_cpt_t*> object_internal_map_t;

    class Base {
        public:
            Base() {
                #ifdef DSPENGINE_HAVE_DEBUG
                    if (__count) {
                        ++__objects_count;
                    }           
                #endif
            }

            Base(const Base &other) {
                #ifdef DSPENGINE_HAVE_DEBUG
                    if (__count) {
                        ++__objects_count;
                    }           
                #endif  
            }

            static const char *__class_name() { return "Object"; } // this method return the class name
            virtual const char *class_name() const { return __class_name(); }

            // enable/disable class instances counting
            // param flag the counting status to set
            static void set_count(bool flag);
            static bool count_active() { return __count; }
            static int objects_count() { return __objects_count; }

            // output the full objects map to a given ostream
            // param out the ostream to write to
            // param map Object map to print out. Per default the current
            // object map __objects_map will be used.
            static void write_objects_map_to( std::ostream& out, object_map_t* map = nullptr );
		    static void write_objects_map_to_cerr() { Base::write_objects_map_to( std::cerr ); }  // output objects map to stderr

            static int bootstrap(Logger *logger, bool count=false);
            //static Logger *logger() { return __logger; }

            static QString base_clock(const QString &sMsg);
            static QString base_clock_in(const QString &sMsg);

            static int getAliveObjectCount();

            static object_map_t getObjectsMap();

            static void printObjectMapDiff(const object_map_t &map);

            static QString sPrintIndention;

            virtual QString toString(const QString &sPrefix = "", bool bShort = true) const;
            
            void Print(bool bShort = true) const;

            void logBacktrace() const;

        protected:
            ~Base() {
                #ifdef DSPENGINE_HAVE_DEBUG
                    if (__count) {
                        --__objects_count;
                    }           
                #endif  
            }

            static bool __count;
            static bool bLogColors;
            static void registerClass(const char *name, const atomic_obj_cpt_t *counters);
            //static TimePoint m_lastTimePoint;

        private:
            static std::atomic<int> __object_count;
            static object_internal_map_t __objects_map;
            static pthread_mutex_t __mutex;

    };

    std::ostream &operator<<(std::ostream &os, const Base &object);
    std::ostream &operator<<(std::ostream &os, const Base *object);

    inline QDebug operator<<(QDebug d, Base *o) {
        d << ( o ? o->toString( "", true ) : "(nullptr)" );
	    return d;
    }

    inline QDebug operator<<( QDebug d, std::shared_ptr<Base> o ) {
        d << ( o ? o->toString( "", true ) : "(nullptr)" );
        return d;
    }
};