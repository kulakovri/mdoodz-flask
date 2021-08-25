from flask import Flask
from flask_cors import CORS
import mdoodz

app = Flask(__name__)
app.config['CORS_HEADERS'] = 'Content-Type'
CORS(app, resources={r"/*": {"origins": "*"}}, supports_credentials=True)
app.register_blueprint(mdoodz.views.blueprint)
