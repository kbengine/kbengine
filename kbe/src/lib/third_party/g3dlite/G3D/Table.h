/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef G3D_TABLE_H
#define G3D_TABLE_H

#include "platform.h"
#include "Array.h"
#include "debug.h"
#include "System.h"
#include "g3dmath.h"
#include "Crypto.h"
#include <assert.h>
#include <string>
#include <locale>

#ifdef G3D_WIN32
#   pragma warning (push)
    // Debug name too long warning
#   pragma warning (disable : 4786)
#endif

namespace G3D {
class Hashable {
public:
    virtual size_t hashCode() const = 0;
    
    /**
     An empty virtual destructor for virtual methods.
    */
    virtual ~Hashable() {}
};
}

/**
 Int hashing function for use with Table.
 */
inline size_t hashCode(const int a) {
	return static_cast<size_t>(a);
}

inline size_t hashCode(const uint32 a) {
	return static_cast<size_t>(a);
}

inline size_t hashCode(const uint64 a) {
	return static_cast<size_t>(a);
}

inline size_t hashCode(const void* a) {
	// Avoid 64-bit pointer cast problems by turning
	// the pointer itself into an array of integers.
	//int* intPtr = (int*)(((unsigned char*)&a) + (sizeof(void*) - sizeof(int)));
	//return *intPtr;

	// sucks - burlex :P
	// use a 64bit hash code instead !
	// void* and size_t should be equal types (4byte on x86, 8byte on x64)
	return reinterpret_cast<size_t>(a);
}

/**
 Default class pointer hash.
 */
inline size_t hashCode(const G3D::Hashable* a) {
    return a->hashCode();
}

/**
 Default class hash.
 */
inline size_t hashCode(const G3D::Hashable& a) {
    return a.hashCode();
}

/**
 String hashing function for use with Table.
 */
inline size_t hashCode(const std::string& a) {
    //static const std::collate<char>& col = std::use_facet< std::collate<char> > (std::locale::empty( ));
    //return col.hash(a.c_str(), a.c_str() + a.size());

	// burlex - this shouldn't be done with only 4 bytes, collisions are possible
	// although i don't think we use std::strings much in this class
    return static_cast<size_t>(G3D::Crypto::crc32(a.c_str(), a.size()));
}

namespace G3D {


/**
 An unordered data structure mapping keys to values.

 Key must be a pointer, an int, a std::string, a class with a hashCode() method,
 or provide overloads for the following <B>two</B> functions: 

  <PRE>
   size_t hashCode(const Key&);
   bool operator==(const Key&, const Key&);
  </PRE>

 G3D pre-defines hash functions for common types (like <CODE>int</CODE> and <CODE>std::string</CODE>).
 If you use a Table with a different type you must write those functions yourself.  For example,
 an enum would use:

  <PRE>
  size_t hashCode(const MyEnum& x) { return (unsigned int)x; };
  </PRE>

  And rely on the default enum operator==.


  Periodically check that debugGetLoad() is low (> 0.1).  When it gets near
  1.0 your hash function is badly designed and maps too many inputs to
  the same output.
 */
template<class Key, class Value> 
class Table {
public:

    /**
     The pairs returned by iterator.
     */
    class Entry {
    public:
        Key    key;
        Value  value;
    };

private:
    /**
     Linked list nodes used internally by HashTable.
     */
    class Node {
    public:
        size_t            hashCode;
        Entry             entry;
        Node*             next;

        
        /** Provide pooled allocation for speed. */
        inline void* operator new (size_t size) {
            return System::malloc(size);
        }
        
        inline void operator delete (void* p) {
            System::free(p);
        }


        Node(Key key, Value value, size_t hashCode, Node* next) {
            this->entry.key   = key;
            this->entry.value = value;
            this->hashCode    = hashCode;
            this->next        = next;
        }

        /**
        Clones a whole chain;
        */
        Node* clone() {
           return new Node(this->entry.key, this->entry.value, hashCode, (next == NULL) ? NULL : next->clone());
        }
    };


    /**
     Number of elements in the table.
     */
    size_t  _size;

    /**
     Array of Node*. 
     We don't use Array<Node*> because Table is lower level.
     Some elements may be NULL.
     */
    Node**  bucket;
    
    /**
     Length of the bucket array.
     */
    size_t  numBuckets;

    /**
     Re-hashes for a larger bucket size.
     */
    void resize(size_t numBuckets) {

        Node** oldBucket = bucket;
        bucket = (Node**)System::alignedMalloc(sizeof(Node*) * numBuckets, 16);
        System::memset(bucket, 0, sizeof(Node*) * numBuckets);

        for (size_t b = 0; b < this->numBuckets; b++) {
            Node* node = oldBucket[b];
         
            while (node != NULL) {
                Node* nextNode = node->next;
        
                // insert at the head of the list for bucket[i]
                size_t i = node->hashCode % numBuckets;
                node->next = bucket[i];
                bucket[i] = node;
        
                node = nextNode;
            }
        }

        System::alignedFree(oldBucket);
        this->numBuckets = numBuckets;
    }

    void copyFrom(const Table<Key, Value>& h) {
        this->_size = h._size;
        this->numBuckets = h.numBuckets;
        this->bucket = (Node**)System::alignedMalloc(sizeof(Node*) * numBuckets, 16);
        System::memset(this->bucket, 0, sizeof(Node*) * numBuckets);
        for (size_t b = 0; b < this->numBuckets; b++) {
            if (h.bucket[b] != NULL) {
                bucket[b] = h.bucket[b]->clone();
            }
        }
    }

    /**
     Frees the heap structures for the nodes.
     */
    void freeMemory() {
        for (size_t b = 0; b < numBuckets; b++) {
           Node* node = bucket[b];
           while (node != NULL) {
                Node* next = node->next;
                delete node;
                node = next;
           }
        }
        System::alignedFree(bucket);
        bucket     = NULL;
        numBuckets = 0;
        _size     = 0;
    }

public:

    /**
     Creates an empty hash table.  This causes some heap allocation to occur.
     */
    Table() {
        numBuckets = 10;
        _size      = 0;
        bucket     = (Node**)System::alignedMalloc(sizeof(Node*) * numBuckets, 16);
        System::memset(bucket, 0, sizeof(Node*) * numBuckets);
    }

    /**
    Destroys all of the memory allocated by the table, but does <B>not</B>
    call delete on keys or values if they are pointers.  If you want to
    deallocate things that the table points at, use getKeys() and Array::deleteAll()
    to delete them.
    */
    virtual ~Table() {
        freeMemory();
    }

    Table(const Table<Key, Value>& h) {
        this->copyFrom(h);
    }

    Table& operator=(const Table<Key, Value>& h) {
        // No need to copy if the argument is this
        if (this != &h) {
            // Free the existing nodes
            freeMemory();
            this->copyFrom(h);
        }
        return *this;
    }

    /**
     Returns the length of the deepest bucket.
     */
    size_t debugGetDeepestBucketSize() const {
        size_t deepest = 0;

        for (size_t b = 0; b < numBuckets; b++) {
            size_t  count = 0;
            Node*   node = bucket[b];
            while (node != NULL) {
                node = node->next;
                ++count;
            }

            if (count > deepest) {
                deepest = count;
            }
        }

        return deepest;
    }

    /**
     A small load (close to zero) means the hash table is acting very
     efficiently most of the time.  A large load (close to 1) means 
     the hash table is acting poorly-- all operations will be very slow.
     A large load will result from a bad hash function that maps too
     many keys to the same code.
     */
    double debugGetLoad() const {
        return debugGetDeepestBucketSize() / (double)size();
    }

    /**
     Returns the number of buckets.
     */
    size_t debugGetNumBuckets() const {
        return numBuckets;
    }

    /**
     C++ STL style iterator variable.  See begin().
     */
    class Iterator {
    private:
        friend class Table<Key, Value>;

        /**
         Bucket index.
         */
        size_t             index;

        /**
         Linked list node.
         */
        Node*              node;
        Table<Key, Value>* table;
        size_t             numBuckets;
        Node**             bucket;
        bool               isDone;

        /**
         Creates the end iterator.
         */
        Iterator(const Table<Key, Value>* table) : table(const_cast<Table<Key, Value>*>(table)) {
            isDone = true;
        }

        Iterator(const Table<Key, Value>* table, size_t numBuckets, Node** bucket) :
            table(const_cast<Table<Key, Value>*>(table)),
            numBuckets(numBuckets),
            bucket(bucket) {
            
            if (numBuckets == 0) {
                // Empty table
                isDone = true;
                return;
            }

            index = 0;
            node = bucket[index];
            isDone = false;
            findNext();
        }

        /**
         Finds the next element, setting isDone if one can't be found.
         Looks at the current element first.
         */
        void findNext() {
            while (node == NULL) {
                index++;
                if (index >= numBuckets) {
                    isDone = true;
                    break;
                } else {
                    node = bucket[index];
                }
            }
        }

    public:
        inline bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }

        bool operator==(const Iterator& other) const {
            if (other.isDone || isDone) {
                // Common case; check against isDone.
                return (isDone == other.isDone) && (other.table == table);
            } else {
                return
                    (table == other.table) &&
                    (node == other.node) && 
                    (index == other.index);
            }
        }

        /**
         Pre increment.
         */
        Iterator& operator++() {
            node = node->next;
            findNext();
            return *this;
        }

        /**
         Post increment (slower than preincrement).
         */
        Iterator operator++(int) {
            Iterator old = *this;
            ++(*this);
            return old;
        }

        const Entry& operator*() const {
            return node->entry;
        }

        Entry* operator->() const {
            return &(node->entry);
        }

        operator Entry*() const {
            return &(node->entry);
        }
    };


    /**
     C++ STL style iterator method.  Returns the first Entry, which 
     contains a key and value.  Use preincrement (++entry) to get to
     the next element.  Do not modify the table while iterating.
     */
    Iterator begin() const {
        return Iterator(this, numBuckets, bucket);
    }

    /**
     C++ STL style iterator method.  Returns one after the last iterator
     element.
     */
    const Iterator end() const {
        return Iterator(this);
    }

    /**
     Removes all elements
     */
    void clear() {
         freeMemory();
         numBuckets = 20;
         _size = 0;
         bucket = (Node**)System::alignedMalloc(sizeof(Node*) * numBuckets, 16);
         System::memset(bucket, 0, sizeof(Node*) * numBuckets);
    }

   
    /**
     Returns the number of keys.
     */
    inline size_t size() const {
        return _size;
    }


    /**
     If you insert a pointer into the key or value of a table, you are
     responsible for deallocating the object eventually.  Inserting 
     key into a table is O(1), but may cause a potentially slow rehashing.
     */
    void set(const Key& key, const Value& value) {
        size_t code = ::hashCode(key);
        size_t b = code % numBuckets;
        
        // Go to the bucket
        Node* n = bucket[b];

        // No bucket, so this must be the first
        if (n == NULL) {
            bucket[b] = new Node(key, value, code, NULL);
            ++_size;
            return;
        }

        size_t bucketLength = 1;

        // Sometimes a bad hash code will cause all elements
        // to collide.  Detect this case and don't rehash when 
        // it occurs; nothing good will come from the rehashing.
        bool allSameCode = true;

        // Try to find the node
        do {
            allSameCode = allSameCode && (code == n->hashCode);

            if ((code == n->hashCode) && (n->entry.key == key)) {
               // Replace the existing node.
               n->entry.value = value;
               return;
            }

            n = n->next;
            ++bucketLength;
        } while (n != NULL);

        const size_t maxBucketLength = 5;
        if ((bucketLength > maxBucketLength) & ! allSameCode && (numBuckets < _size * 20)) {
            // This bucket was really large; rehash if all elements
            // don't have the same hashcode the number of buckets is reasonable.
            resize(numBuckets * 2 + 1);
        }

        // Not found; insert at the head.
        b = code % numBuckets;
        bucket[b] = new Node(key, value, code, bucket[b]);
        ++_size;
   }

   /**
    Removes an element from the table if it is present.  It is an error
    to remove an element that isn't present.
    */
   void remove(const Key& key) {
      
      size_t code = ::hashCode(key);
      size_t b = code % numBuckets;

      // Go to the bucket
      Node* n = bucket[b];

      // Make sure it was found
      alwaysAssertM(n != NULL, "Tried to remove a key that was not in the table.");

      Node* previous = NULL;

      // Try to find the node
      do {
          if ((code == n->hashCode) && (n->entry.key == key)) {
              // This is the node; remove it
              if (previous == NULL) {
                  bucket[b] = n->next;
              } else {
                  previous->next = n->next;
              }
              // Delete the node
              delete n;
              --_size;
              return;
          }

          previous = n;
          n = n->next;
       } while (n != NULL);


      alwaysAssertM(false, "Tried to remove a key that was not in the table.");
   }

   /**
    Returns the value associated with key.
    @deprecated Use get(key, val) or 
    */
   Value& get(const Key& key) const {

      size_t code = ::hashCode(key);
      size_t b = code % numBuckets;

      Node* node = bucket[b];

      while (node != NULL) {
         if ((node->hashCode == code) && (node->entry.key == key)) {
            return node->entry.value;
         }
         node = node->next;
      }

      debugAssertM(false, "Key not found");
      // The next line is here just to make
      // a compiler warning go away.
      return node->entry.value;
   }

   /**
    If the key is present in the table, val is set to the associated value and returns true.
    If the key is not present, returns false.
    */
   bool get(const Key& key, Value& val) const {
	  size_t code = ::hashCode(key);
      size_t b = code % numBuckets;

      Node* node = bucket[b];

      while (node != NULL) {
          if ((node->hashCode == code) && (node->entry.key == key)) {
             // found key
             val = node->entry.value;
             return true;
          }
          node = node->next;
      }

      // Failed to find key
      return false;
   }

   /**
    Returns true if key is in the table.
    */
   bool containsKey(const Key& key) const {
       size_t code = ::hashCode(key);
       size_t b = code % numBuckets;

       Node* node = bucket[b];

       while (node != NULL) {
           if ((node->hashCode == code) && (node->entry.key == key)) {
              return true;
           }
           node = node->next;
       } while (node != NULL);

       return false;
   }


   /**
    Short syntax for get.
    */
   inline Value& operator[](const Key &key) const {
      return get(key);
   }


   /**
    Returns an array of all of the keys in the table.
    You can iterate over the keys to get the values.
    */
   Array<Key> getKeys() const {
       Array<Key> keyArray;
       getKeys(keyArray);
       return keyArray;
   }

   void getKeys(Array<Key>& keyArray) const {
       keyArray.resize(0, DONT_SHRINK_UNDERLYING_ARRAY);
       for (size_t i = 0; i < numBuckets; i++) {
           Node* node = bucket[i];
           while (node != NULL) {
               keyArray.append(node->entry.key);
               node = node->next;
           }
       }
   }

   /**
    Calls delete on all of the keys.  Does not clear the table, 
    however, so you are left with a table of dangling pointers.

    Same as <CODE>getKeys().deleteAll();</CODE>

    To delete all of the values, you may want something like
    <PRE>
        Array<Key> keys = table.getKeys();
        Set<Value> value;
        for (size_t k = 0; k < keys.length(); k++) {
           value.insert(keys[k]);
        }
        value.getMembers().deleteAll();
        keys.deleteAll();
    </PRE>
    */
   void deleteKeys() {
       getKeys().deleteAll();
   }

   /**
    Calls delete on all of the values.  This is unsafe--
    do not call unless you know that each value appears
    at most once.

    Does not clear the table, so you are left with a table
    of dangling pointers.
    */
   void deleteValues() {
       for (size_t i = 0; i < numBuckets; i++) {
           Node* node = bucket[i];
           while (node != NULL) {
               delete node->entry.value;
               node = node->next;
           }
       }
   }
};

} // namespace

#endif
#ifdef G3D_WIN32
#   pragma warning (pop)
#endif
