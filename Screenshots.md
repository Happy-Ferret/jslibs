

# Fire effect #

<a href='http://www.youtube.com/watch?feature=player_embedded&v=vuOxQFvQwLU' target='_blank'><img src='http://img.youtube.com/vi/vuOxQFvQwLU/0.jpg' width='425' height=344 /></a>

### source ###
```
...
texture.Set(0);
texture.ForEachPixel(function(pixel, x, y) {

 var displace = PerlinNoise(1.9, 0.4, 5, x/2, y/2 + z*1.5);
 var val = PerlinNoise(2, 0.5, 3, (x + displace*40)/6, (y + z + displace*160+(texture.height-y))/8);
 val *= y / texture.height * 1.5;
 pixel[0] = val;
 pixel[1] = val*val*val;
});
z += 0.5;
...
```

[complete source code](http://code.google.com/p/jslibs/source/browse/trunk/src/jsprotex/debug.js#1)

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)

---

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)
# Cloud effect #

<a href='http://www.youtube.com/watch?feature=player_embedded&v=3zQyFeR70ss' target='_blank'><img src='http://img.youtube.com/vi/3zQyFeR70ss/0.jpg' width='425' height=344 /></a>

### source ###
```
...
texture.Set(1);
texture.ForEachPixel(function(pixel, x, y) {

 var dis = PerlinNoise(2.1, 0.5, 5, x-z/2, y);
 var val = PerlinNoise(1.8, 0.6, 7, x-z+dis*8, y);
 pixel[0] = pixel[1] = 1-val;
});
z += 1;
...
```

[complete source code](http://code.google.com/p/jslibs/source/browse/trunk/src/jsprotex/debug.js#1)

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)

---

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)
# jsdebug module #

![http://jslibs.googlecode.com/svn/wiki/jslibs_debugger.png](http://jslibs.googlecode.com/svn/wiki/jslibs_debugger.png)

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)


---

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)
# jsgraphics module #

![http://jslibs.googlecode.com/svn/wiki/screenshot-jsgraphics-001.png](http://jslibs.googlecode.com/svn/wiki/screenshot-jsgraphics-001.png)
![http://jslibs.googlecode.com/svn/wiki/screenshot-jsgraphics-002.png](http://jslibs.googlecode.com/svn/wiki/screenshot-jsgraphics-002.png)

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)

---

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)
# jsio module #

![http://jslibs.googlecode.com/svn/wiki/screenshot-jsnspr-001.png](http://jslibs.googlecode.com/svn/wiki/screenshot-jsnspr-001.png)
![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)

---

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)
# jsfont module + jsprotex module #

![http://jslibs.googlecode.com/svn/wiki/screenshot-jsfont+jsprotex-helloWorld.png](http://jslibs.googlecode.com/svn/wiki/screenshot-jsfont+jsprotex-helloWorld.png)
![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)

---

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)