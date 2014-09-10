#!/bin/bash

SRCDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"
RELDIR="$SRCDIR/tools/visualizer"
COMPRESS="java -jar $SRCDIR/tools/yuicompressor-2.4.6.jar"

mkdir -p $RELDIR/{css,js} 
mkdir -p $SRCDIR/builds

cat $SRCDIR/js/modernizr.js $SRCDIR/js/jquery/{jquery,jquery.event.drag,jquery.mousewheel,jquery.svgscrollview,slick.grid,slick.model}.js \
    | $COMPRESS --type js -o $RELDIR/js/libraries.js
cat $SRCDIR/js/gooda/{State,Notification,Columns,FileLoader,EventTable,GraphPane,DataView,FunctionView,HotspotView,ContainerView,HelpPane,Report,Visualizer,load}.js \
    | $COMPRESS --type js -o $RELDIR/js/visualizer.js

cat $SRCDIR/css/{slick.grid,grid.theme,visualizer}.css \
    | $COMPRESS --type css -o $RELDIR/css/visualizer.css

cp -R $SRCDIR/{images,reports,tools/index.html} $RELDIR
cp -R $SRCDIR/java $RELDIR
cp -R $SRCDIR/js/{dijit,dojo,dojox} $RELDIR/js
tar --exclude *.uncompressed.js --exclude .DS_Store -czf $SRCDIR/builds/visualizer$(date +"%m%d").tar.gz -C $SRCDIR/tools/ visualizer
rm -rf $RELDIR
