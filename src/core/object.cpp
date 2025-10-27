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

#include "object.h"
extern "C" {
    #include "core/logger.h"
}

#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <typeinfo>
#include <atomic>
#include <map>
#include <pthread.h>

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

namespace PCore {   
    void Base::set_count(bool flag) {
        #ifdef DSPENGINE_HAVE_DEBUG
            __count = flag;
        #endif
    }

    void Base::write_objects_map_to(std::ostream &out, object_map_t *map) {
        #ifndef DSPENGINE_HAVE_DEBUG
            object_map_t snapshot;
            if (map == nullptr) {
                snapshot = getObjectsMap();
                map = &snapshot;
            }

            std::ostringstream o;
            pthread_mutex_lock( &__mutex );
            object_map_t::iterator it = map->begin();
            while ( it != map->end() ) {
                if ( it->second.constructed || it->second.destructed ) {
                    o << "\t[ " << std::setw( 30 ) << ( *it ).first << " ]\t" << std::setw( 6 ) << ( *it ).second.constructed << "\t" << std::setw( 6 ) << ( *it ).second.destructed
                    << "\t" << std::setw( 6 ) << ( *it ).second.constructed - ( *it ).second.destructed << std::endl;
                }
                it++;
            }
            pthread_mutex_unlock( &__mutex );

            out << std::endl;
            if ( Base::bLogColors ) {
                out << "\033[35m";
            }

            out << "Objects map :" << std::setw( 30 ) << "class\t" << "constr   destr   alive" << std::endl << o.str() << "Total : " << std::setw( 6 ) << __object_count << " objects.";

            if ( Base::bLogColors ) {
                out << "\033[0m";
            }
            out << std::endl << std::endl;
        #else
            if ( ! Base::bLogColors ) {
                out << "Base::write_objects_map_to :: not compiled with H2CORE_HAVE_DEBUG flag set" << std::endl;
            } else {
                out << "\033[35mBase::write_objects_map_to :: \033[31mnot compiled with H2CORE_HAVE_DEBUG flag set\033[0m" << std::endl;
            }
        #endif
    }

    int Base::getAliveObjectCount() {
        #ifdef DSPENGINE_HAVE_DEBUG
            int nCount = 0;
            for (const auto& ii : __objects_map ) {
                if (!strcmp(ii.first, "Object")) {
                    return ii.second->constructed - ii.second->destructed;
                }
            }
	        return nCount;
        #else
	        return 0;
        #endif
    }

    object_map_t Base::getObjectsMap() {
        object_map_t mapCopy;
        obj_cpt_t copy;

        for (auto const &ii : __objects_map) {
            copy.constructed = ii.second->constructed;
            copy.destructed = ii.second->destructed;
            mapCopy.insert(std::pair<const char*, obj_cpt_t>(ii.first, copy));
        }

        return mapCopy;
    }

    void Base::printObjectMapDiff(const object_map_t &mapSnapshot) {
        object_map_t mapDiff;
        obj_cpt_t diff;

        // Since key value pairs are only inserted but not erased while
        // a subset of the current object map.
        for ( const auto& ii : __objects_map ) {
            auto it = mapSnapshot.find( ii.first );
            if ( it != mapSnapshot.end() ) {
                diff.constructed = ii.second->constructed - it->second.constructed;
                diff.destructed = ii.second->destructed - it->second.destructed;
                mapDiff.insert( std::pair<const char*, obj_cpt_t>( ii.first, diff ) );
            }
        }

        write_objects_map_to( std::cout, &mapDiff );
    }

    bool Base::__count = false;                         // default: disabled
    bool Base::bLogColors = true;                       // default: colors on
    std::atomic<int> Base::__object_count{0};           // object count
    object_internal_map_t Base::__objects_map{};        // map<const char*, const atomic_obj_cpt_t*>
    pthread_mutex_t Base::__mutex = PTHREAD_MUTEX_INITIALIZER;
};