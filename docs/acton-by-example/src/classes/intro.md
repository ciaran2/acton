# Classes and Objects

Acton supports the object-oriented paradigm, which means that it provides features to create classes and objects, which are instances of classes. Classes are a fundamental concept in an object-oriented world and they allow programmers to create their own data types with their own attributes and methods.

A class is defined using the `class` keyword followed by the name of the class. The convention is to use CamelCase for class names.

```python
class Circle(object):
    radius: float

    def __init__(self, radius):
        self.radius = radius

    def diameter(self):
        return self.radius * 2
```


*Attributes* are variables that hold data for an object of a particular class and *methods* are functions that operate on that data. In the above example, `radius` is an attribute of the `Circle` class and `diameter()` is a method that returns the diameter.

Class methods must have `self` as the first argument, which refers to the object instance of the class that the method is called on.

## Creating an object 

A Class is like a blueprint and an object is an instance of such a blueprint. To create an object, or "instantiate", we use the "blueprint" (class), like so:

```python
circle = Circle(3.14)
```

Here we create the object `circle` from the class `Circle`, passing the parameter `3.14` which will be used to set the `radius` attribute of the object.

```python
print(circle.diameter())
```

And here we print the diameter of the circle by calling the `.diameter()` method.
