from django.conf.urls import url
from . import views

urlpatterns = [
    url(r'^$', views.manage, name='manage'),
    url(r'^get/$', views.get_req, name='get_req'),
]

