## Лабораторная работа №4

# Запуск и компиляция
## Динамические библиотеки
### 1. Перед началом работы необходимо скомпилировать библиотеки

```
gcc -o liblibrary_1.so -shared -fPIC library_1.c
gcc -o liblibrary_2.so -shared -fPIC library_2.c
```

- **shared** компонует динамическую библиотеку вместо исполняемого файла
- **fPIC** создаёт PLT и GOT сегменты и патчит адреса в этих секциях, чтобы вызовы функций ссылались на пропатченные адреса в таблице


### 2. Скомпилировать программу, которая реалиует логику динамической загрузки библиотеки во время runtime

```
gcc dynamic.c -o dynamic.out
```

### 3. Скомпилировать программу, которая использует динамическую библиотеку во время линковки

```
gcc static.c ./liblibrary_1.so -o static_1.out
gcc static.c ./liblibrary_2.so -o static_2.out
```

## Статические библиотеки
### 1. Скомпилировать объектный файл библиотеки

```
gcc -c library_1.c -o library_1.o
gcc -c library_2.c -o library_2.o
```

### 2. Создать статическую библиотеку

```
ar rc liblibrary_1.a library_1.o
ar rc liblibrary_2.a library_2.o
```

- **r** - заменить или добавить файлы в архиве
- **c** - создать архив, если не существует

### 3. Скомпилировать программу, которая реализует логику загрузки библиотеки во время компиляции

```
gcc -o static_1.out static.c liblibrary_1.a
gcc -o static_2.out static.c liblibrary_2.a
```

## Запуск

```
./dynamic.out 2 ./liblibrary_1.so ./liblibrary_2.so 
./static_1.out
./static_2.out
```