# SPDX-License-Identifier: AGPL-3.0-or-later

from .resource import (
    json, _start_end,
)

from ..utils import (
    _server_store, _folder, HTTPBadRequest
)

from .folder import FolderResource

class CalendarResource(FolderResource):
    fields = FolderResource.fields.copy()
    fields.update({
        'displayName': lambda folder: folder.name,
    })

    def on_get(self, req, resp, userid=None, folderid=None, method=None):
        server, store = _server_store(req, userid, self.options)

        folder = _folder(store, folderid or 'calendar')

        if method == 'calendarView':
            start, end = _start_end(req)
            def yielder(**kwargs):
                for occ in folder.occurrences(start, end, **kwargs):
                    yield occ
            data = self.generator(req, yielder)
            fields = EventResource.fields

        elif method == 'events':
            data = self.generator(req, folder.items, folder.count)
            fields = EventResource.fields

        elif method:
            raise HTTPBadRequest("Unsupported segment '%s'" % method)

        else:
            data = folder
            fields = None

        self.respond(req, resp, data, fields)

    def on_post(self, req, resp, userid=None, folderid=None, method=None):
        server, store = _server_store(req, userid, self.options)
        folder = store.calendar # TODO

        if method == 'events':
            fields = json.loads(req.stream.read().decode('utf-8'))
            item = self.create_message(folder, fields, EventResource.set_fields)
            item.send()
            self.respond(req, resp, item, EventResource.fields)

from .event import (
    EventResource
)
