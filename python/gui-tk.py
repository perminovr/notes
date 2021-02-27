#!/usr/bin/python3
import PySimpleGUI as sg

# Very basic form. Return values as a list
form = sg.FlexForm('Простая форма ввода')  # begin with a blank form

layout = [
	[sg.Text('Тут запрос неких данных на вход')],
	[sg.Text('Ввод текста', size=(15, 1)), sg.InputText('username')],
	[sg.Text('Выбор пункта', size=(15, 1)), sg.InputCombo(('Значение1', 'Значение 2'), size=(20, 3))],
	[sg.Checkbox('Чекбокс1'), sg.Checkbox('Чекбокс2', default=True)],
	[sg.Text('_'  * 80)],
	[sg.Text('Указать путь', size=(15, 1), auto_size_text=False, justification='right'), sg.InputText('~/'), sg.FolderBrowse()],
	[sg.Submit(), sg.Cancel()]
]

button, values = form.Layout(layout).Read()
