#!/bin/sh

product_dir=product

mkdir -p $product_dir
mkdir -p $product_dir/Resources
mkdir -p $product_dir/bin

echo -n "Copying files"

cp maptiler $product_dir/bin

echo -n "."

cp -r configs $product_dir/Configurations

echo -n "."

cp tiles/* $product_dir/Resources

echo -n "."

cp *.png *.ttf *.m4v $product_dir/Resources

echo "done!"
