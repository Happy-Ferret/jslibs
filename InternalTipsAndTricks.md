Ensure an element is visible:
```
		function ElementAbsolutePosition(elt) [(function(o) o.offsetLeft + (o.offsetParent ? arguments.callee(o.offsetParent) : 0))(elt), (function(o) o.offsetTop + (o.offsetParent ? arguments.callee(o.offsetParent) : 0))(elt)];

		function SetInView(elt, offset) {

			var pos = ElementAbsolutePosition(elt)[1];
			var body = elt.ownerDocument.body;
			if ( pos < body.scrollTop || pos + elt.offsetHeight > body.scrollTop + body.clientHeight ) {
			
				elt.scrollIntoView();
				body.scrollTop += offset || 0;
			}
		}
```

---

```
		function StringReplacer(conversionObject) function(s) s.replace(new RegExp([k.replace(/(\/|\.|\*|\+|\?|\||\(|\)|\[|\]|\{|\}|\\)/g, "\\$1") for (k in conversionObject)].join('|'), 'g'), function(str) conversionObject[str]);

		var EscapeEntities = StringReplacer({'&':'&amp;', '<':'&lt;', '>':'&gt;'});

```

---

Wrapper to XPath for XHTML or XUL documents:
```
		function XPath(ref, xpath) {
			
			doc = ref.ownerDocument || ref;
			var result = doc.evaluate(xpath, ref, null, XPathResult.ANY_TYPE, null);
			switch (result.resultType) {

				case XPathResult.NUMBER_TYPE:
					return result.numberValue;
				case XPathResult.BOOLEAN_TYPE:
					return result.booleanValue;
				case XPathResult.STRING_TYPE:
					return result.stringValue;
			} // else XPathResult.UNORDERED_NODE_ITERATOR_TYPE
			var list = [];
			for ( var node = result.iterateNext(); node; node = result.iterateNext() )
				switch ( node.nodeType ) {
				case node.ATTRIBUTE_NODE:
					list.push( node.value );
					break;
				case node.TEXT_NODE:
					list.push( node.data );
					break;
				default:
					list.push( node );
				}
			return list;
		}

```

---

```
		function TextToHTMLNode(doc, text) {
			
			var elt = doc.createElement('div');
			elt.innerHTML = text;
			return elt.firstChild;
		}

		function InsertNode(node, pos) pos.parentNode.insertBefore(node, pos);
		
		function InsertNodeAtBeginning(node, parent) parent.firstChild ? parent.insertBefore(node, parent.firstChild) : parent.appendChild(node);

```

---

HTML class handling:
```
		function HasClass(elt, class) (' '+elt.className+' ').indexOf(' '+class+' ') != -1;

		function SetClass(polarity, elt, class) {
		
			class = ' '+class+' ';
			var str = ' '+elt.className+' ';
			var pos = str.lastIndexOf(class);
			if (polarity != (pos != -1))
				elt.className = polarity ? str+class : str.substr(0, pos+1)+str.substr(pos+class.length);
		}
```

---


XUL inline CSS stylesheet:
```
<?xml-stylesheet href="data:text/css,
...
" type="text/css"?>
```

---

```
```

---

```
```

---

```
```

---

```
```

---

```
```

---

```
```

---

```
```

---

```
```

---

```
```

---

```
```