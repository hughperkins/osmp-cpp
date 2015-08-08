// Copyright Hugh Perkins 2005
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURVector3E. See the GNU General Public License for
//  more details.
//
// You should have received a copy of the GNU General Public License along
// with this program in the file licence.txt; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-
// 1307 USA
// You can find the licence also on the web at:
// http://www.opensource.org/licenses/gpl-license.php
//

// directorin is for passing arguments into a callback function 
// called from C++
%typemap(python, directorin) const char * CALLBACKINPUT{
   
   $input = PyString_FromString( CALLBACKINPUT );
}

%typemap(python, directorout) const char * {
     cout << "directorout" << endl;
}

%typemap(in) string & {
   /* Check if is a string */
   if (PyString_Check($input)) {
      //$1 = (string *)malloc( sizeof( string * ) );
      $1 = new string( PyString_AsString( $input ) );
   } else {
      PyErr_SetString(PyExc_TypeError,"not a string");
      return NULL;
   }
}

%typemap(freearg) string & {
   delete( $1 );
   //free( $1 );
}

%typemap(in) string {
   /* Check if is a string */
   if (PyString_Check($input)) {
      $1 = string( PyString_AsString( $input ) );
   } else {
      PyErr_SetString(PyExc_TypeError,"not a string");
      return NULL;
   }
}

%typemap(freearg) string {
   //delete( $1 );
}

%typemap(python,out) string {
   $target = PyString_FromString( $source.c_str() );
}

%typemap(python,ret) string {
}

%typemap(in) char ** {
   /* Check if is a list */
   if (PyList_Check($input)) {
      int size = PyList_Size($input);
      int i = 0;
      $1 = (char **) malloc((size+1)*sizeof(char *));
      for (i = 0; i < size; i++) {
         PyObject *o = PyList_GetItem($input,i);
         if (PyString_Check(o))
            $1[i] = PyString_AsString(PyList_GetItem($input,i));
         else {
            PyErr_SetString(PyExc_TypeError,"list must contain strings");
            free($1);
            return NULL;
         }
      }
      $1[i] = 0;
   } else {
      PyErr_SetString(PyExc_TypeError,"not a list");
      return NULL;
   }
}
// This cleans up the char ** array we malloc'd before the function call
%typemap(freearg) char ** {
   free( (char *) $1);
}

%typemap(in) char **& {
   /* Check if is a list */
   if (PyList_Check($input)) {
      int size = PyList_Size($input);
      int i = 0;
    //  cout << "allocating array, for " << size << " elements" << endl;
      $1 = (char ***)malloc( sizeof( char *));
     // cout << "allocated pointer to pointer" << endl;
      *($1) = (char **) malloc((size+1)*sizeof(char *));
    //  cout << "assinged to array variable" << endl;
      char **array = *$1;      
    //  cout << "got array" << endl;
      for (i = 0; i < size; i++) {
         PyObject *o = PyList_GetItem($input,i);
         if (PyString_Check(o)) {
           //  cout << "adding string " << PyString_AsString(PyList_GetItem($input,i)) << endl;
            array[i] = PyString_AsString(PyList_GetItem($input,i));
         }
         else {
            PyErr_SetString(PyExc_TypeError,"list must contain strings");
            free(array);
            free($1);
            return NULL;
         }
      }
      array[i] = 0;
   //   cout << "done" << endl;
   } else {
      PyErr_SetString(PyExc_TypeError,"not a list");
      return NULL;
   }
}
// This cleans up the char ** array we malloc'd before the function call
%typemap(freearg) char **& {
   //   cout << "freeing mem" << endl;
   free( (char *) (*$1));
   free( (char *) ($1));
    //  cout << "done" << endl;
}

// convert List[] to vector<int> &
%typemap(in) vector<int> &(vector<int> tempvec) {
   if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyInt_Check(o)) {
        tempvec.push_back( PyInt_AsLong( o) );
        $1 = &tempvec;
      }
      else {
        PyErr_SetString(PyExc_TypeError,"list must contain integers");
	     return NULL;
      }
    }
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a list.");
    return NULL;
  }
}

// convert vector<int> to List[]; note that swig actually include std_vector.i
// but I didnt know that when I wrote this.  We might switch to use Swig's at some 
// point, though for now this seems to work just fine
%typemap(out) vector<int> () {
  int i;
  $result = PyList_New($1.size());
  vector<int>::iterator it;
   i = 0;
   for( it = $1.begin(); it != $1.end(); it++ )
   {
      PyObject *o = PyInt_FromLong( *it );
      PyList_SetItem($result,i,o);
      i++;
   }
}

// Used to append arguments to the list of returned values
// in Python
// You'll need to instantiate this for each class you want to use in your .i file
// eg: argoutfn( MyClassName )
// Note that %defines in Swig are multi-line: they end at the %enddef marker
// Note: for this to work, operator= must work correctly for the class
%define argoutfn( ClassName )
%typemap(argout) ClassName &{
    PyObject *oArgToReturn, *oOldReturnValue, *o3;
    
//    cout << "Blah value is " << $1->GetValue() << endl;
    ClassName *preturnedblah = new ClassName;
    *preturnedblah = *$1;
  //  cout << "new Blah value is " << preturnedblah->GetValue() << endl;    
    oArgToReturn = SWIG_NewPointerObj( preturnedblah, $1_descriptor, 1 );
    
    if ((!$result) || ($result == Py_None)) {
    //    cout << "No result before, so just returning oArgToReturn" << endl;
        $result = oArgToReturn;
    } else {
        if (!PyTuple_Check($result)) {
            PyObject *oOldReturnValue = $result;
            $result = PyTuple_New(1);
            PyTuple_SetItem($result,0,oOldReturnValue);
        }
        o3 = PyTuple_New(1);
        PyTuple_SetItem(o3,0,oArgToReturn);
        oOldReturnValue = $result;
        $result = PySequence_Concat(oOldReturnValue,o3);
        Py_DECREF(oOldReturnValue);
        Py_DECREF(o3);
    }
}

%typemap(in,numinputs=0) ClassName &(ClassName temp ) {
    $1 = &temp;
}

%enddef
