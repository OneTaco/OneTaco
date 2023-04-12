all:ObjectManager
ObjectManager:ObjectManager.c main0.c
	gcc ObjectManager.c main0.c -o ObjectManager
clean:
	rm -rf ObjectManager
