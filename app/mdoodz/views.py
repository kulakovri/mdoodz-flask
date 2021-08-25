from flask import Blueprint

blueprint = Blueprint('mdoodz', __name__)


@blueprint.get('/mdoodz')
def index():
    print('test')
    return "This is an example app"
