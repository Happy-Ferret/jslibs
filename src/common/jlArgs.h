/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */


#pragma once


JL_BEGIN_NAMESPACE


#define ARGSARGS cx, argc, vp

struct Args {

	JS::PersistentRootedObject _thisObj; // use HandleObject instead ?
	const JS::CallArgs _jsargs;
	JSContext *&_cx;

	Args(JSContext *&cx, unsigned argc, JS::Value *&vp)
	: _cx(cx), _thisObj(cx), _jsargs( JS::CallArgsFromVp(argc, vp) ) {
	}

	JSObject *callee() {

		return &_jsargs.callee();
	}

	unsigned length() const {

		return _jsargs.length();
	}

	bool hasDefined(unsigned i) const {

		return _jsargs.hasDefined(i);
	}

	JS::MutableHandleValue handleAt(unsigned i) const {

		return _jsargs[i];
	}

	JS::HandleValue handleOrUndefinedAt(unsigned i) const {

		return _jsargs.get(i);
	}

	JS::MutableHandleValue rval() const {

		return _jsargs.rval();
	}

	bool isConstructing() const {

		return _jsargs.isConstructing();
	}

	void constructThis(JSClass *clasp, JS::HandleObject proto) {

		ASSERT( isConstructing() );
		_thisObj.set( jl::newObjectWithGivenProto(_cx, clasp, proto) ); // JS_NewObjectForConstructor() use the callee to determine parentage and [[Prototype]].
		rval().setObject(*_thisObj);
		_jsargs.setThis(rval());
	}

	void computeThis() {

		JS::Value tmp = _jsargs.thisv();
		if ( tmp.isObject() ) {

			_thisObj.set( &tmp.toObject() );
		} else
		if ( tmp.isNullOrUndefined() ) {

			_thisObj.set( JL_GetGlobal(_cx) ); //_thisObj.set( JS_GetGlobalForObject(_cx, &_jsargs.callee() ) );
		} else {

			if ( !JS_ValueToObject(_cx, _jsargs.thisv(), &_thisObj) ) {

				_thisObj.set(NULL);
			}
		}
		_jsargs.setThis(JS::ObjectValue(*_thisObj));
	}

	JS::HandleObject thisObj() {

		if ( !_thisObj )
			computeThis();
		return JS::HandleObject::fromMarkedLocation(_thisObj.address());
	}

	JS::HandleValue thisObjVal() {

		if ( _jsargs.thisv().isObject() )
			return _jsargs.thisv();
		computeThis();
		return _jsargs.thisv();
	}

private:
	void operator=( const Args & );
};


#define PROPARGSARGS cx, obj, id, vp

struct PropArgs {

	JS::HandleObject &_thisObj;
	JS::MutableHandleValue &_vp;

	PropArgs(JSContext *cx, JS::HandleObject &obj, JS::HandleId id, JS::MutableHandleValue &vp)
	: _thisObj(obj), _vp(vp) {
	}

	unsigned length() const {

		return 1;
	}

	bool hasDefined(unsigned) const {

		return true;
	}

	JS::MutableHandleValue &handleAt(unsigned) {

		return _vp;
	}

	JS::HandleValue handleOrUndefinedAt(unsigned) const {

		return _vp;
	}

	JS::MutableHandleValue &rval() {

		return _vp;
	}

	JS::HandleObject thisObj() const {

		return _thisObj;
	}

private:
	void operator=( const PropArgs & );
};


JL_END_NAMESPACE



#define JL_ARGC (args.length())

// returns the ARGument n
#define JL_ARG( n ) (ASSERT((n) > 0 && (unsigned)(n) <= JL_ARGC), args.handleAt((n)-1))

// returns the ARGument n or undefined if it does not exist
#define JL_SARG( n ) (args.handleOrUndefinedAt((n)-1))

// returns true if the ARGument n IS DEFined
#define JL_ARG_ISDEF( n ) (args.hasDefined((n)-1))

// the return value
#define JL_RVAL (args.rval())

// is the current obj (this)
#define JL_OBJ (args.thisObj())

// is the current obj (this) as a JS::Value. if this method returns null, an error has occurred and must be propagated or caught.
#define JL_OBJVAL (args.thisObjVal())


#define JL_DEFINE_ARGS \
	jl::Args args(ARGSARGS);

#define JL_DEFINE_PROP_ARGS \
	jl::PropArgs args(PROPARGSARGS);

#define JL_DEFINE_CALL_FUNCTION_OBJ \
	JS::RootedObject obj(cx, JS_CALLEE(cx, vp).toObjectOrNull();

#define JL_DEFINE_CONSTRUCTOR_OBJ \
	JL_MACRO_BEGIN \
	args.constructThis(JL_THIS_CLASS, jl::Host::getHost(cx).getCachedProto(JL_THIS_CLASS_NAME)); \
	JL_MACRO_END


/*
// test:
#define JL_ARG_GEN(N, type) \
	TYPE arg##N; \
	if ( JL_ARG_ISDEF(N) ) \
		JL_CHK( jl::getValue(cx, JL_ARG(n), &arg##N) ); \
*/

