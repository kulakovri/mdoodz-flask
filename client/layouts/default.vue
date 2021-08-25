<template>
  <v-app>
    <ErrorAlert :error="error"></ErrorAlert>
    <v-navigation-drawer app fixed>
      <v-list>
        <v-list-item
          v-for="(item, i) in items"
          :key="i"
          :to="item.to"
          router
          exact
        >
          <v-list-item-action>
            <v-icon>{{ item.icon }}</v-icon>
          </v-list-item-action>
          <v-list-item-content>
            <v-list-item-title v-text="item.title" />
          </v-list-item-content>
        </v-list-item>
      </v-list>
    </v-navigation-drawer>
    <v-app-bar :clipped-left="clipped" fixed app flat>
      <v-toolbar-title class="text-capitalize" v-text="title" />
      <v-spacer />
      <v-menu bottom :offset-y="true">
      </v-menu>
    </v-app-bar>
    <v-main>
      <v-container>
        <nuxt />
      </v-container>
    </v-main>
  </v-app>
</template>

<script>
import ErrorAlert from '~/components/common/alerts/ErrorAlert';
import navigation from '~/plugins/mixins/navigation';
import { userStore } from '~/store';

export default {
  components: {
    ErrorAlert,
  },
  mixins: [navigation],
  data() {
    return {
      clipped: true,
      drawer: true,
      fixed: true,
      items: [
        {
          icon: 'mdi-home',
          title: 'Home',
          to: '/home',
        },
      ],
      miniVariant: false,
    };
  },
  computed: {
    title() {
      return this.$route.name;
    },
    firstName() {
      return userStore.firstName;
    },
    email() {
      return userStore.email;
    },
    error() {
      return userStore.error;
    },
  },
};
</script>
