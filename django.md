## Django at a glance

### Design your model

Although you can use Django without a database, it comes with an object-relational mapper in which you describe your database layout in Python code.

Here's a quick example:
```
# mysite/news/models.py

from django.db import models

class Reporter(models.Model):
    full_name = models.CharField(max_length=70)

    def __str__(self):              # __unicode__ on Python 2
        return self.full_name

class Article(models.Model):
    pub_date = models.DateField()
    headline = models.CharField(max_length=200)
    content = models.TextField()
    reporter = models.ForeignKey(Reporter, on_delete=models.CASCADE)

    def __str__(self):              # __unicode__ on Python 2
        return self.headline
```

#### Install it
Next, run the Django command-line utility to create the database tables automatically:
```
$ python manage.py migrate
```
The migrate command looks at all your available models and creates tables in your database for whichever tables don't already exist, as well as optionally providing much richer schema control.

### Design your URLs
To design URLs for an app, you create a Python module called a URLconf. A table of contents for your app, it contains a simple mapping between URL patterns and Python callback functions. URLconfs also serve to decouple URLs from Python code.

Here’s what a URLconf might look like for the Reporter/Article example above:
```
# mysite/news/urls.py

from django.conf.urls import url

from . import views

urlpatterns = [
    url(r'^articles/([0-9]{4})/$', views.year_archive),
    url(r'^articles/([0-9]{4})/([0-9]{2})/$', views.month_archive),
    url(r'^articles/([0-9]{4})/([0-9]{2})/([0-9]+)/$', views.article_detail),
]
```
The code above maps URLs, as simple regular expressions, to the location of Python callback functions ("views"). The regular expressions use parenthesis to "capture" values from the URLs. When a user requests a page, Django runs through each pattern, in order, and stops at the first one that matches the requested URL. (If none of them matches, Django calls a special-case 404 view.) 

Once one of the regexes matches, Django imports and calls the given view, which is a simple Python function. Each view gets passed a request object – which contains request metadata – and the values captured in the regex.

For example, if a user requested the URL “/articles/2005/05/39323/”, Django would call the function `news.views.article_detail(request, '2005', '05', '39323')`.

### Write your views
Each view is responsible for doing one of two things: Returning an HttpResponse object containing the content for the requested page, or raising an exception such as Http404. The rest is up to you.

Generally, a view retrieves data according to the parameters, loads a template and renders the template with the retrieved data. Here's an example view for year_archive from above:
```
# mysite/news/views.py

from django.shortcuts import render

from .models import Article

def year_archive(request, year):
    a_list = Article.objects.filter(pub_date__year=year)
    context = {'year': year, 'article_list': a_list}
    return render(request, 'news/year_archive.html', context)
```

### Design your templates
The code above loads the news/year_archive.html template.

Django has a template search path, which allows you to minimize redundancy among templates. In your Django settings, you specify a list of directories to check for templates with DIRS. If a template doesn’t exist in the first directory, it checks the second, and so on.

Let’s say the news/year_archive.html template was found. Here’s what that might look like:
```
<!--mysite/news/templates/news/year_archive.html-->

{% extends "base.html" %}

{% block title %}Articles for {{ year }}{% endblock %}

{% block content %}
<h1>Articles for {{ year }}</h1>

{% for article in article_list %}
    <p>{{ article.headline }}</p>
    <p>By {{ article.reporter.full_name }}</p>
    <p>Published {{ article.pub_date|date:"F j, Y" }}</p>
{% endfor %}
{% endblock %}
```
Variables are surrounded by double-curly braces. `{{ article.headline }}` means "Output the value of the article’s headline attribute." But dots aren't used only for attribute lookup.

Note `{{ article.pub_date|date:"F j, Y" }}` uses a Unix-style "pipe" (the "|"" character). This is called a template filter, and it's a way to filter the value of a variable.

## Writing your first Django app, part 1
Throughout this tutorial, we'll walk you through the creation of a basic poll application.

### Creating a project
If this is your first time using Django, you'll have to take care of some initial setup. Namely, you’ll need to auto-generate some code that establishes a Django project – a collection of settings for an instance of Django, including database configuration, Django-specific options and application-specific settings.

From the command line, `cd` into a directory where you'd like to store your code, then run the following command:
```
$ django-admin startproject mysite
```
This will create a mysite directory in your current directory.

Let’s look at what `startproject` created:
```
mysite/
    manage.py
    mysite/
        __init__.py
        settings.py
        urls.py
        wsgi.py
```
These files are:
- The outer `mysite/` root directory is just a container for your project. Its name doesn't matter to Django; you can rename it to anything you like.
- mysite/settings.py: Settings/configuration for this Django project.
- mysite/urls.py: The URL declarations for this Django project; a "table of contents" of your Django-powered site.

### The development server
Let's verify your Django project works. Change into the outer `mysite` directory, if you haven't already, and run the following commands:
```
$ python manage.py runserver
```
You've started the Django development server, a lightweight Web server written purely in Python. We've included this with Django so you can develop things rapidly, without having to deal with configuring a production server – such as Apache – until you're ready for production.

Now that the server's running, visit http://127.0.0.1:8000/ with your Web browser. You'll see a "Welcome to Django" page, in pleasant, light-blue pastel. It worked!

#### Changing the port

By default, the runserver command starts the development server on the internal IP at port 8000.

If you want to change the server's port, pass it as a command-line argument. For instance, this command starts the server on port 8080:
```
$ python manage.py runserver 8080
```
If you want to change the server's IP, pass it along with the port. For example, to listen on all available public IPs, use:
```
$ python manage.py runserver 0:8000
```
0 is a shortcut for 0.0.0.0.

### Creating the Polls app
Now that your environment – a "project" – is set up, you're set to start doing work.

To create your app, make sure you're in the same directory as `manage.py` and type this command:
```
python manage.py startapp polls
```
That'll create a directory `polls`, which is laid out like this:
```
polls/
    __init__.py
    admin.py
    apps.py
    migrations/
        __init__.py
    models.py
    tests.py
    views.py
```
This directory structure will house the poll application.

### Write your first view
Let's write the first view. Open the file polls/views.py and put the following Python code in it:
```
# polls/views.py
from django.http import HttpResponse

def index(request):
    return HttpResponse("Hello, world. You're at the polls index.")
```
To call the view, we need to map it to a URL - and for this we need a URLconf.
To create a URLconf in the polls directory, create a file called `urls.py`. Your app directory should now look like:
```
polls/
    __init__.py
    admin.py
    apps.py
    migrations/
        __init__.py
    models.py
    tests.py
    urls.py
    views.py
```
In the polls/urls.py file include the following code:
```
# polls/urls.py

from django.conf.urls import url

from . import views

urlpatterns = [
    url(r'^$', views.index, name='index'),
]
```
The next step is to point the root URLconf at the polls.urls module.
```
# mysite/urls.py

from django.conf.urls import include, url
from django.contrib import admin

urlpatterns = [
    url(r'^polls/', include('polls.urls')),
    url(r'^admin/', admin.site.urls),
]
```
The `include()` function allows referencing other URLconfs.

You have now wired an index view into the URLconf. Lets verify it's working, run the following command:
```
$ python manage.py runserver
```
Go to http://localhost:8000/polls/ in your browser, and you should see the text "Hello, world. You're at the polls index.", which you defined in the index view.

## Writing your first Django app, part 2
### Database setup

Now, open up mysite/settings.py. It's a normal Python module with module-level variables representing Django settings.

By default, the configuration uses SQLite.

If you wish to use another database, install the appropriate database bindings and change the following keys in the `DATABASES 'default'` item to match your database connection settings:

- `ENGINE` – Either `'django.db.backends.sqlite3'`, `'django.db.backends.postgresql'`, or `'django.db.backends.mysql'`.
- `NAME` – The name of your database. If you’re using SQLite, the database will be a file on your computer; in that case, `NAME` should be the full absolute path, including filename, of that file.

```
$ python manage.py migrate
```
The `migrate` command looks at the `INSTALLED_APPS` setting and creates any necessary database tables according to the database settings in your `mysite/settings.py` file and the database migrations shipped with the app. You’ll see a message for each migration it applies. If you're interested, run the command-line client for your database and type `\dt` (PostgreSQL), `SHOW TABLES;` (MySQL), or `.schema` (SQLite) to display the tables Django created.

### Creating models
A model is the single, definitive source of information about your data. It contains the essential fields and behaviors of the data you're storing. Generally, each model maps to a single database table.

The basics:
- Each model is a Python class that subclasses `django.db.models.Model`.
- Each attribute of the model represents a database field.

In our simple poll app, we'll create two models: `Question` and `Choice`. A `Question` has a question and a publication date. A `Choice` has two fields: the text of the choice and a vote tally. Each `Choice` is associated with a `Question`.

These concepts are represented by simple Python classes. Edit the `polls/models.py` file so it looks like this:
```
# polls/models.py
from django.db import models


class Question(models.Model):
    question_text = models.CharField(max_length=200)
    pub_date = models.DateTimeField('date published')


class Choice(models.Model):
    question = models.ForeignKey(Question, on_delete=models.CASCADE)
    choice_text = models.CharField(max_length=200)
    votes = models.IntegerField(default=0)
```
The code is straightforward. Each model is represented by a class that subclasses `django.db.models.Model`. Each model has a number of class variables, each of which represents a database field in the model.

### Activating models
That small bit of model code gives Django a lot of information. With it, Django is able to:
- Create a database schema (`CREATE TABLE` statements) for this app.
- Create a Python database-access API for accessing `Question` and `Choice` objects.

## Writing your first Django app, part 3
We're continuing the Web-poll application and will focus on creating the public interface – "views."
### Overview
A view is a "type" of Web page in your Django application that generally serves a specific function and has a specific template.

In Django, web pages and other content are delivered by views. Each view is represented by a simple Python function (or method, in the case of class-based views). Django will choose a view by examining the URL that's requested.

A URL pattern is simply the general form of a URL - for example: `/newsarchive/<year>/<month>/`.

To get from a URL to a view, Django uses what are known as 'URLconfs'. A URLconf maps URL patterns (described as regular expressions) to views.

### Writing more views
Now let's add a few more views to `polls/views.py`. These views are slightly different, because they take an argument:
```
# polls/views.py

def detail(request, question_id):
    return HttpResponse("You're looking at question %s." % question_id)

def results(request, question_id):
    response = "You're looking at the results of question %s."
    return HttpResponse(response % question_id)

def vote(request, question_id):
    return HttpResponse("You're voting on question %s." % question_id)
```
Wire these new views into the `polls.urls` module by adding the following `url()` calls:
```
# polls/urls.py

from django.conf.urls import url

from . import views

urlpatterns = [
    # ex: /polls/
    url(r'^$', views.index, name='index'),
    # ex: /polls/5/
    url(r'^(?P<question_id>[0-9]+)/$', views.detail, name='detail'),
    # ex: /polls/5/results/
    url(r'^(?P<question_id>[0-9]+)/results/$', views.results, name='results'),
    # ex: /polls/5/vote/
    url(r'^(?P<question_id>[0-9]+)/vote/$', views.vote, name='vote'),
]
```
Take a look in your browser, at "/polls/34/". It'll run the `detail()` method and display whatever ID you provide in the URL. Try "/polls/34/results/" and "/polls/34/vote/" too – these will display the placeholder results and voting pages.

When somebody requests a page from your website – say, "/polls/34/", Django will load the `mysite.urls` Python module because it's pointed to by the `ROOT_URLCONF` setting. It finds the variable named `urlpatterns` and traverses the regular expressions in order. After finding the match at `'^polls/'`, it strips off the matching text (`"polls/"`) and sends the remaining text – `"34/"` – to the 'polls.urls' URLconf for further processing. There it matches `r'^(?P<question_id>[0-9]+)/$'`, resulting in a call to the `detail()` view like so:
```
detail(request=<HttpRequest object>, question_id='34')
```
The `question_id='34'` part comes from `(?P<question_id>[0-9]+)`. Using parentheses around a pattern "captures" the text matched by that pattern and sends it as an argument to the view function.

### Write views that actually do something
Each view is responsible for doing one of two things: returning an HttpResponse object containing the content for the requested page, or raising an exception such as Http404. The rest is up to you.

Your view can read records from a database, or not. It can use a template system such as Django's.
Because it's convenient, let's use Django's own database API. Here's one stab at a new `index()` view, which displays the latest 5 poll questions in the system, separated by commas, according to publication date:
```
# polls/views.py

from django.http import HttpResponse

from .models import Question


def index(request):
    latest_question_list = Question.objects.order_by('-pub_date')[:5]
    output = ', '.join([q.question_text for q in latest_question_list])
    return HttpResponse(output)

# Leave the rest of the views (detail, results, vote) unchanged
```
There's a problem here, though: the page's design is hard-coded in the view. If you want to change the way the page looks, you'll have to edit this Python code. So let's use Django's template system to separate the design from Python by creating a template that the view can use.

First, create a directory called `templates` in your `polls` directory. Django will look for templates in there.

Within the `templates` directory you have just created, create another directory called `polls`, and within that create a file called `index.html`. In other words, your template should be at `polls/templates/polls/index.html`.

Put the following code in that template:
```
<!--polls/templates/polls/index.html-->

{% if latest_question_list %}
    <ul>
    {% for question in latest_question_list %}
        <li><a href="/polls/{{ question.id }}/">{{ question.question_text }}</a></li>
    {% endfor %}
    </ul>
{% else %}
    <p>No polls are available.</p>
{% endif %}
```

Now let's update our `index` view in `polls/views.py` to use the template:
```
# polls/views.py

from django.http import HttpResponse
from django.template import loader

from .models import Question


def index(request):
    latest_question_list = Question.objects.order_by('-pub_date')[:5]
    template = loader.get_template('polls/index.html')
    context = {
        'latest_question_list': latest_question_list,
    }
    return HttpResponse(template.render(context, request))
```
That code loads the template called `polls/index.html` and passes it a context.