from django import forms

class ManageForm(forms.Form):
    command = forms.CharField(
              label = 'commnad',
              max_length = 50,
              widget = forms.TextInput()
              )

class MessageForm(forms.Form):
    message = forms.CharField(
              label = 'message',
              max_length = 50,
              widget = forms.TextInput()
              )

